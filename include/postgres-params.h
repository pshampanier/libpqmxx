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

#include <string>

namespace db {
  namespace postgres {

    class Params {

      friend class Connection;

    public:
      Params(int size) {
        size_ = size;
        if (size_ > 0) {
          types_ = new Oid[size];
          values_ = new char *[size];
          lengths_ = new int[size];
          formats_ = new int[size];
        }
      }

      ~Params() {
        delete [] types_;
        delete [] values_;
        delete [] lengths_;
        delete [] formats_;
      }

      void bind() const {}

      void bind(int i);
      void bind(const char *sz);
      void bind(const std::string &s);

    private:
      int    size_;
      int    index_ = 0;
      Oid   *types_ = nullptr;
      char **values_ = nullptr;
      int   *lengths_ = nullptr;
      int   *formats_ = nullptr;
    };

  } // namespace postgres
}   // namespace db
