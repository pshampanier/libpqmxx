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

#include "postgres-types.h"

#include <string>
#include <vector>

namespace db {
  namespace postgres {

    /**
     * A private class to bind SQL command parameters.
     **/
    class Params {

      friend class Connection;

    private:
      std::vector<Oid>      types_;
      std::vector<char *>   values_;
      std::vector<int>      lengths_;
      std::vector<int>      formats_;
      std::vector<char *>   buffers_;
      const struct Settings &settings_;

      Params(const Settings &settings, int size);
      ~Params();

      char *bind(Oid type, size_t length);

      void bind() const {}
      void bind(std::nullptr_t);
      void bind(const std::string &s);
      void bind(const std::vector<uint8_t> &bytes);

      template<typename T>
      void bind(T v);

      void bind(Oid type, char *value, size_t length);

      template <typename T>
      T *bind(Oid type, size_t length);

      /**
       * Arrays
       **/
      template<typename T>
      void bind(const std::vector<array_item<T>> &array);

      template<typename T>
      void bind(Oid type, Oid elemType, const std::vector<array_item<T>> &array);
    };

  } // namespace postgres
}   // namespace db
