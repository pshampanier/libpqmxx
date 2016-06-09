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
    
    void bind(Connection &c, int i) {
      return;
    }
    
    void bind(Connection &c, const std::string &s) {
      return;
    }
        
    // -------------------------------------------------------------------------
    // Destructor.
    // -------------------------------------------------------------------------
    Connection::~Connection() {
      PQfinish(conn_);
    }
    
    // -------------------------------------------------------------------------
    // Open a connection to the database.
    // -------------------------------------------------------------------------
    Connection &Connection::connect(const char *connInfo, bool async) {
      assert(conn_ == nullptr);
      async_ = async;
      conn_ = PQconnectdb(connInfo);
      
      if( PQstatus(conn_) != CONNECTION_OK ) {
        std::string err(PQerrorMessage(conn_));
        PQfinish(conn_);
        conn_ = nullptr;
        throw ConnectionException(err);
      }
      
      if (PQsetnonblocking(conn_, async_) != 0) {
        // ## TODO
        return *this;
      }
      
      return *this;
    }
    
    // -------------------------------------------------------------------------
    // Close the database connection.
    // -------------------------------------------------------------------------
    void Connection::close() noexcept {
      assert(conn_);
      PQfinish(conn_);
      conn_ = nullptr;
    }
    
    // -------------------------------------------------------------------------
    // Get the native socket identifier.
    // -------------------------------------------------------------------------
    int Connection::socket() const noexcept {
      return PQsocket(conn_);
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