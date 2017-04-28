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
#include "boost/asio.hpp"
#pragma GCC diagnostic pop

namespace db {
namespace postgres {
namespace async_boost {
  
  using namespace ::db::postgres;

  /**
   * An asynchronous implementation of a postgresql connection based on boost.
   **/
  class Connection : public postgres::Connection {
  public:

    Connection(::boost::asio::io_service& ioService)
    : socket_(ioService) {
      async_ = true;
    }
    
    virtual ~Connection() override {
      if (socket_.is_open()) {
        socket_.cancel();
      }
    }
    
    // ---------------------------------------------------------------------
    // Trigger an action to the layer in charge of asynchonous I/O.
    // ---------------------------------------------------------------------
    virtual void asyncAction(action action, handler_t callback) override {
      
      switch (action) {
          
        //
        // Connection to the server establised.
        //
        case postgres::action::connect:
          socket_.assign(::boost::asio::ip::tcp::v4(), socket());
          break;
          
        //
        // Connection to the server closed.
        //
        case postgres::action::close:
          socket_.cancel();
          break;
          
        //
        // Connection reset to the server initiated.
        //
        case postgres::action::reset:
          
          break;
          
        //
        // Connection read pending.
        //
        case postgres::action::read:
          socket_.async_read_some(::boost::asio::null_buffers(),
            [this, callback](::boost::system::error_code ec, size_t) {
              std::exception_ptr eptr;
              if (ec && ec != ::boost::asio::error::operation_aborted) {
                eptr = make_exception_ptr(std::runtime_error(ec.message()));
              }
              callback(eptr);
            }
          );
          break;
          
        //
        // Connection write pending.
        //
        case postgres::action::write:
          socket_.async_write_some(::boost::asio::null_buffers(),
            [this, callback](::boost::system::error_code ec, size_t) {
              std::exception_ptr eptr;
              if (ec && ec != ::boost::asio::error::operation_aborted) {
                eptr = make_exception_ptr(std::runtime_error(ec.message()));
              }
              callback(eptr);
            }
          );
          break;
          
      }
      
    }
    
  private:
    ::boost::asio::ip::tcp::socket socket_;
  };

} // namespace postgres
} // namespace db
} // namespace async_boost
