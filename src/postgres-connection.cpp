/**
 * Copyright (c) 2016 Philippe FERDINAND
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/
#include "postgres-connection.h"
#include "postgres-exceptions.h"

#include <functional>
#include <cassert>
#include <string>
#include <cstring>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Secur32.lib")

namespace db {
  namespace postgres {
    
    void consumeInput(ResultHandle *rh);
    void attach(ResultHandle *rh, PGresult *pgr);
    void setLastError(ResultHandle *rh, std::exception_ptr eptr);

    typedef std::function<void (const char *)> notice_handler;

    // -------------------------------------------------------------------------
    // Notice processor (dispatch the notice to the registered handler).
    // -------------------------------------------------------------------------
    static void noticeProcessor(void *arg, const char *message) {
      notice_handler *notice = (notice_handler *)arg;
      if (notice) {
        (*notice)(message);
      }
    }

    // -------------------------------------------------------------------------
    // Constructor.
    // -------------------------------------------------------------------------
    Connection::Connection(Settings settings) {
      pgconn_ = nullptr;
      async_ = false;
      singleRowMode_ = false;
      handler_ = nullptr;
      settings_ = settings;
    }

    // -------------------------------------------------------------------------
    // Destructor.
    // -------------------------------------------------------------------------
    Connection::~Connection() {
      PQfinish(pgconn_);
    }
    
    // -------------------------------------------------------------------------
    // Set the notice handler
    // -------------------------------------------------------------------------
    Connection &Connection::notice(std::function<void (const char *)> callback) noexcept {
      notice_handler *notice = callback ? &callback : nullptr;
      PQsetNoticeProcessor(pgconn_, noticeProcessor, notice);
      return *this;
    }

    // -------------------------------------------------------------------------
    // Open a connection to the database.
    // -------------------------------------------------------------------------
    Connection &Connection::connect(const char *connInfo, handler_t handler) {
      connInfo = connInfo == nullptr ? "" : connInfo;
      if (async_) {
        assert(handler);
        handler_ = handler;
        pgconn_ = PQconnectStart(connInfo);
        if (pgconn_) {
          auto status = PQstatus(pgconn_);
          if (status == CONNECTION_BAD) {
            auto eptr = std::make_exception_ptr(error(error_code::connection_failure, lastPostgresError()));
            connectCompleted(eptr);
          }
          else {
            PQsetnonblocking(pgconn_, 1);
            asyncAction(action::connect, [this](std::exception_ptr &eptr) {
              if (!eptr) {
                // The postgresql documentation very explicit about the fact we
                // should wait for the socket to be write ready:
                //
                // «If you have yet to call PQconnectPoll, i.e., just after the
                //  call to PQconnectStart, behave as if it last returned
                //  PGRES_POLLING_WRITING.»
                //
                // Using boost, when the connection is refused the write event
                // never comes. Instead boost gives an error on the read.
                asyncAction(action::read_write, [this](std::exception_ptr eptr) {
                  connectPoll();
                });
              }
              else {
                connectCompleted(eptr);
              }
            });
          }
        }
      }
      else {
        pgconn_ = PQconnectdb(connInfo);
        if( PQstatus(pgconn_) != CONNECTION_OK ) {
          std::string lastError = lastPostgresError();
          PQfinish(pgconn_);
          pgconn_ = nullptr;
          throw error(error_code::connection_failure, lastError);
        }
      }

      return *this;
    }
    
    // -------------------------------------------------------------------------
    // Close the database connection.
    // -------------------------------------------------------------------------
    Connection &Connection::close() noexcept {
      if (pgconn_) {
        asyncAction(action::close, [this](std::exception_ptr &) {
          // Any error during the close is ignored.
          PQfinish(pgconn_);
          pgconn_ = nullptr;
        });
      }
      return *this;
    }

    // -------------------------------------------------------------------------
    // Last error message on the connection.
    // -------------------------------------------------------------------------
    std::string Connection::lastPostgresError() const noexcept {
      return std::string(PQerrorMessage(pgconn_));
    }

    // -------------------------------------------------------------------------
    // Execute an SQL statement.
    // -------------------------------------------------------------------------
    Result Connection::execute(const char *sql, const Params &params) {
      
      Result result(*this);
      if (async_) {
        lastResult_ = result.handle_;
      }
      
      try {
        int success = PQsendQueryParams(pgconn_, sql, int(params.values_.size()),
                                        params.types_.data(),
                                        params.values_.data(),
                                        params.lengths_.data(),
                                        params.formats_.data(),
                                        1 /* binary results */);
        
        if (singleRowMode_) {
          singleRowMode_ = false;
          if (success) {
            PQsetSingleRowMode(pgconn_);
          }
        }

        if (success) {
          if (async_) {
            flush();
          }
          else {
            result.first();
          }
        }

        if (!success) {
          throw error(error_code::unknown, lastPostgresError());
        }
      }
      catch (...) {
        setLastError(result.handle_.get(), std::current_exception());
      }
      
      return result;
    }
    
    // -------------------------------------------------------------------------
    // Execute a batch of SQL commands.
    // -------------------------------------------------------------------------
    Result Connection::execute(const BatchStatement &sql) {
      
      Result result(*this);
      
      try {
        if (async_) {
          int success = PQsendQuery(pgconn_, sql);
          if (success) {
            lastResult_ = result.handle_;
            flush();
          }
          else {
            throw error(error_code::unknown, lastPostgresError());
          }
        }
        else {
          attach(result.handle_.get(), PQexec(pgconn_, sql));
        }
      }
      catch (...) {
        setLastError(result.handle_.get(), std::current_exception());
      }
      
      return result;
      
    }
    
    // -------------------------------------------------------------------------
    // Execute the next statement in single row mode.
    // -------------------------------------------------------------------------
    Connection &Connection::setSingleRowMode() noexcept {
      singleRowMode_ = true;
      return *this;
    }
    
    // -------------------------------------------------------------------------
    // Cancel queries in progress.
    // -------------------------------------------------------------------------
    Connection &Connection::cancel() {
      char errbuf[256];
      PGcancel *pgcancel = PQgetCancel(pgconn_);
      if (pgcancel == nullptr) {
        throw std::runtime_error("Cancel operation on an invalid connection.");
      }
      else {
        int success = PQcancel(pgcancel, errbuf, sizeof(errbuf));
        if (!success) {
          throw std::runtime_error(errbuf);
        }
        PQfreeCancel(pgcancel);
      }
      return *this;
    }

    // -------------------------------------------------------------------------
    // Process available data retreived from the server.
    // -------------------------------------------------------------------------
    void Connection::consumeInput() {
      
      if (pgconn_) {
        auto lastResult = lastResult_.lock();
        asyncAction(action::read, [this, lastResult](std::exception_ptr eptr) {
          if (eptr && lastResult) {
            setLastError(lastResult.get(), eptr);
          }
          else if (pgconn_) {
            if (lastResult) {
              ::db::postgres::consumeInput(lastResult.get());
            }
            consumeInput();
          }
        });
      }
      
    }

    // -------------------------------------------------------------------------
    // Flush the data pending to be send throught the connection.
    // -------------------------------------------------------------------------
    void Connection::flush() {

      int res = PQflush(pgconn_);
      switch (res) {
        case 0:
          // Send queue flushed successfully. Now checking if some data are
          // ready to be consumed.
          consumeInput();
          break;
          
        case 1:
          // More data to send
          asyncAction(action::write, [this](std::exception_ptr eptr) {
            if (eptr) {
              auto lastResult = lastResult_.lock();
              if (lastResult) {
                setLastError(lastResult.get(), eptr);
              }
            }
            else if (pgconn_) {
              this->flush();
            }
          });
          break;
          
        case -1:
          throw error(error_code::unknown, lastPostgresError());
          break;
      }

    }

    // -------------------------------------------------------------------------
    // Check the connection progress status.
    // -------------------------------------------------------------------------
    void Connection::connectPoll() {
      PostgresPollingStatusType status = PQconnectPoll(pgconn_);
      try {
        switch (status) {
          case PGRES_POLLING_FAILED:
            throw error(error_code::connection_failure, lastPostgresError());
            break;

          case PGRES_POLLING_READING:
            asyncAction(action::read, [this](std::exception_ptr eptr) {
              if (eptr) {
                connectCompleted(eptr);
              }
              else {
                connectPoll();
              }
            });
            break;

          case PGRES_POLLING_WRITING: {
            asyncAction(action::read_write, [this](std::exception_ptr eptr) {
              if (eptr) {
                connectCompleted(eptr);
              }
              else {
                connectPoll();
              }
            });
            break;
          }

          case PGRES_POLLING_OK:
            connectCompleted();
            consumeInput();
            break;

          default:
            assert(false);
            break;
        }
      }
      catch(...) {
        connectCompleted(std::current_exception());
      }
    }

    // -------------------------------------------------------------------------
    // Get the native socket identifier.
    // -------------------------------------------------------------------------
    int Connection::socket() const noexcept {
     return PQsocket(pgconn_);
    }
    
    // -------------------------------------------------------------------------
    // Asynchronous connection completed
    // -------------------------------------------------------------------------
    void Connection::connectCompleted(std::exception_ptr eptr) noexcept {
      assert(handler_);
      if (eptr && pgconn_) {
        PQfinish(pgconn_);
        pgconn_ = nullptr;
      }
      handler_t h;
      h.swap(handler_);
      try {
        h(eptr);
      }
      catch (...) {
        // The handler is not supposed to throw any exception.
        assert(true);
      }
    }

    // -------------------------------------------------------------------------
    // Trigger an event to the layer in charge of asynchonous I/O.
    // -------------------------------------------------------------------------
    void Connection::asyncAction(action, handler_t callback) noexcept {
      std::exception_ptr action_ok;
      callback(action_ok);
    }

  } // namespace postgres
}   // namespace db
