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
#pragma once
#include "postgres-connection.h"

#pragma warning( disable : 4068)  // Visual Studio: ignore #pragma GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include <boost/exception/diagnostic_information.hpp>
#include "boost/asio.hpp"
#pragma GCC diagnostic pop

namespace libpqmxx {
namespace async_boost {

  static std::exception_ptr action_ok;

  /**
   * An asynchronous implementation of a postgresql connection based on boost.
   **/
  class Connection : public libpqmxx::Connection {
  public:

    // -------------------------------------------------------------------------
    // Constructor
    // -------------------------------------------------------------------------
    Connection(::boost::asio::io_service& ioService)
      : socket_(ioService) {
      async_ = true;
    }
    
    // -------------------------------------------------------------------------
    // Destructor
    // -------------------------------------------------------------------------
    virtual ~Connection() override {
      close();
    }
    
    // -------------------------------------------------------------------------
    // Trigger an action to the layer in charge of asynchonous I/O.
    // -------------------------------------------------------------------------
    virtual void asyncAction(action action, handler_t callback) noexcept override {

      try {

        auto ignore = std::make_shared<bool>(false);
        auto boostHandler = [this, action, ignore, callback](::boost::system::error_code ec, size_t) {
          if (!*ignore && ec != ::boost::asio::error::operation_aborted) {
            if (action == libpqmxx::action::read_write) {
              // Cancel the other operation to prevent a second call to the handler.
              socket_.cancel();

              // Under Windows, the cancel can be completely ignored and will
              // result on calling twice the handler. This is why we are adding
              // a shared reference to a boolean to the capture. This way we can
              // guaranty to ignore the second unexpected call.
              *ignore = true;
            }
            if (ec) {
              auto eptr = make_exception_ptr(std::runtime_error(ec.message()));
              callback(eptr);
            }
            else {
              callback(action_ok);
            }
          }
        };

        switch (action) {

          //
          // Connection to the server establised.
          //
          case libpqmxx::action::connect:
            socket_.assign(::boost::asio::ip::tcp::v4(), socket());
            callback(action_ok);
            break;

          //
          // Connection to the server closed.
          //
          case libpqmxx::action::close:
            socket_.cancel();
            callback(action_ok);
            break;

          //
          // Connection reset to the server initiated.
          //
          case libpqmxx::action::reset:

            break;

          //
          // Connection read pending.
          //
          case libpqmxx::action::read:
            socket_.async_read_some(::boost::asio::null_buffers(), boostHandler);
            break;

          //
          // Connection write pending.
          //
          case libpqmxx::action::write:
            socket_.async_write_some(::boost::asio::null_buffers(), boostHandler);
            break;

          //
          // Connection read or write pending.
          //
          case libpqmxx::action::read_write: {
            socket_.async_read_some(::boost::asio::null_buffers(), boostHandler);
            socket_.async_write_some(::boost::asio::null_buffers(), boostHandler);
          }

        }
      }
      catch (boost::exception &e) {
        // Because boost exception don't include the diagnostic information,
        // we are creating a new exception for the code boost aware.
        auto eptr = std::make_exception_ptr(std::runtime_error(boost::diagnostic_information(e)));
        callback(eptr);
      }
      catch (...) {
        auto eptr = std::current_exception();
        callback(eptr);
      }
      
    }
    
  private:
    ::boost::asio::ip::tcp::socket socket_;
  };

} // namespace libpqmxx
} // namespace async_boost
