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
    namespace boost {

      /**
       * An asynchronous implementation of a postgresql connection based on boost.
       **/
      class Connection : public db::postgres::Connection {
      public:

        Connection(::boost::asio::io_service& ioService)
        : socket_(ioService) {
          async_ = true;
        }

        // ---------------------------------------------------------------------
        // Close the connection to the database server.
        // ---------------------------------------------------------------------
        virtual Connection &close() override {
          if (readCallback_ || writeCallback_) {
            // Cancel the pending io.
            this->socket_.cancel();
          }

          return Connection::close();
        }

        // ---------------------------------------------------------------------
        // Wait for the next io event.
        // ---------------------------------------------------------------------
        virtual void asyncWait(std::function<void (bool)> readCallback,
                               std::function<void (bool)> writeCallback) override {

          if (socket_.native() == -1) {
            socket_.assign(::boost::asio::ip::tcp::v4(), socket());
          }

          if (readCallback) {
            assert(readCallback_ == nullptr);
            readCallback_ = readCallback;
            socket_.async_read_some(::boost::asio::null_buffers(),
              [this, readCallback, writeCallback](::boost::system::error_code ec, size_t) {
                if (!ec && writeCallback) {
                  // There was also a write operation pending, we need to cancel it.
                  ec = this->socket_.cancel(ec);
                }
                bool aborted = (ec == ::boost::asio::error::operation_aborted);
                if (!ec || aborted) {
                  readCallback_ = nullptr;
                  readCallback(aborted);
                }
                else {
                  // throw an exception
                  this->throwException<std::runtime_error>(ec.message());
                }
              }
            );
          }

          if (writeCallback) {
            assert(writeCallback_ == nullptr);
            writeCallback_ = writeCallback;
            socket_.async_write_some(::boost::asio::null_buffers(),
              [this, readCallback, writeCallback](::boost::system::error_code ec, size_t) {
                if (!ec && readCallback) {
                  // There was also a read operation pending, we need to cancel it.
                  ec = this->socket_.cancel(ec);
                }
                bool aborted = (ec == ::boost::asio::error::operation_aborted);
                if (!ec || aborted) {
                  writeCallback_ = nullptr;
                  writeCallback(aborted);
                }
                else {
                  // throw an exception
                  this->throwException<std::runtime_error>(ec.message());
                }
              }
            );
          }

        }

      private:
        ::boost::asio::ip::tcp::socket socket_;
        std::function<void (bool)>     readCallback_ = nullptr;
        std::function<void (bool)>     writeCallback_ = nullptr;
      };

    } // namespace boost
  }   // namespace postgres
}     // namespace db
