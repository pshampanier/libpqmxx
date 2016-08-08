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
#include <vector>

namespace db {
  namespace postgres {

    template <typename T>
    struct array {
      int32_t ndim; /* Number of dimensions */
      int32_t ign;  /* offset for data, removed by libpq */
      Oid elemtype; /* type of element in the array */

      /* First dimension */
      int32_t size;  /* Number of elements */
      int32_t index; /* Index of first element */
      T first_value; /* Beginning of the data */
    };

    /**
     * A private class to bind SQL command paramters.
     **/
    class Params {

      friend class Connection;

    private:
      std::vector<Oid>     types_;
      std::vector<char *>  values_;
      std::vector<int>     lengths_;
      std::vector<int>     formats_;
      std::vector<char *>  buffers_;

      Params(int size);

      char *bind(Oid type, size_t length);

      void bind() const {}
      void bind(std::nullptr_t);
      void bind(const std::string &s);
      void bind(const std::vector<uint8_t> &d);

      template<typename T>
      void bind(T v);

      template<typename T>
      void bind(Oid elemType, std::vector<T> value);

      void bind(Oid type, char *value, size_t length);

      template <typename T>
      T *bind(Oid type, size_t length);
    };

  } // namespace postgres
}   // namespace db
