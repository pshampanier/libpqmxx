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

namespace db {
  namespace postgres {

    // -------------------------------------------------------------------------
    // Constructor.
    // -------------------------------------------------------------------------
    Connection::Connection()
      : result_(*this) {
    }

    // -------------------------------------------------------------------------
    // Destructor.
    // -------------------------------------------------------------------------
    Connection::~Connection() {
      PQfinish(pgconn_);
    }
    
    // -------------------------------------------------------------------------
    // Open a connection to the database.
    // -------------------------------------------------------------------------
    Connection &Connection::connect(const char *connInfo, bool async) {
      assert(pgconn_ == nullptr);
      async_ = async;
      pgconn_ = PQconnectdb(connInfo);
      
      if( PQstatus(pgconn_) != CONNECTION_OK ) {
        PQfinish(pgconn_);
        pgconn_ = nullptr;
        throw ConnectionException(std::string(PQerrorMessage(pgconn_)));
      }
      
      if (PQsetnonblocking(pgconn_, async_) != 0) {
        // ## TODO
        return *this;
      }

      return *this;
    }
    
    // -------------------------------------------------------------------------
    // Close the database connection.
    // -------------------------------------------------------------------------
    void Connection::close() noexcept {
      assert(pgconn_);
      PQfinish(pgconn_);
      pgconn_ = nullptr;
    }

    // -------------------------------------------------------------------------
    // Last error message on the connection.
    // -------------------------------------------------------------------------
    std::string Connection::lastError() const {
      return std::string(PQerrorMessage(pgconn_));
    }

    // -------------------------------------------------------------------------
    // Execute an SQL statement.
    // -------------------------------------------------------------------------
    void Connection::execute(const char *sql, const Params &params) {

      result_.clear();

      int success = PQsendQueryParams(pgconn_, sql, params.values_.size(),
                                      params.types_.data(),
                                      params.values_.data(),
                                      params.lengths_.data(),
                                      params.formats_.data(),
                                      1 /* binary results */);

      if (!success) {
        throw ExecutionException(lastError());
      }

      // Switch to the single row mode to avoid loading the all result in memory.
      success = PQsetSingleRowMode(pgconn_);
      assert(success);

      result_.first();
    }
    
    // -------------------------------------------------------------------------
    // Cancel queries in progress.
    // -------------------------------------------------------------------------
    Connection &Connection::cancel() {
      // ##
      // ## TO DO
      // ##
      return *this;
    }

    // -------------------------------------------------------------------------
    // Get the native socket identifier.
    // -------------------------------------------------------------------------
    int Connection::socket() const noexcept {
      return PQsocket(pgconn_);
    }
    
    Connection &Connection::once(std::function<bool (const Result &)> cb) {
      iterator_ = cb;
      return *this;
    }
    
    Connection &Connection::each(std::function<bool (const Result &)> cb) {
      iterator_ = cb;
      return *this;
    }
    
    Connection &Connection::done(std::function<void (int)> cb) {
      done_ = cb;
      return *this;
    }
    
    Connection &Connection::always(std::function<void ()> cb) {
      always_ = cb;
      return *this;
    }
    
    // -------------------------------------------------------------------------
    // Error handler registration.
    // -------------------------------------------------------------------------
    Connection &Connection::error(std::function<void (std::exception_ptr)> cb) {
      error_ = cb;
      return *this;
    }

  } // namespace postgres
}   // namespace db
