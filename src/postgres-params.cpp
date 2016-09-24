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

#include <cassert>
#include <cstring>

namespace db {
  namespace postgres {

    //--------------------------------------------------------------------------
    // Constructor
    //--------------------------------------------------------------------------
    Params::Params(const Settings &settings, int size)
    : settings_(settings) {
      types_.reserve(size);
      values_.reserve(size);
      lengths_.reserve(size);
      formats_.reserve(size);
    }

    Params::~Params() {
      for (char *value: buffers_) {
        delete [] value;
      }
    }

    template <typename T>
    char *write(T value, char *buf);

    char *Params::bind(Oid type, size_t length) {
      char *buf = new char[length];
      buffers_.push_back(buf);
      bind(type, buf, length);
      return buf;
    }

    //--------------------------------------------------------------------------
    // Bind any value
    //--------------------------------------------------------------------------
    void Params::bind(Oid type, char *value, size_t length) {
    #ifdef __clang__
      // Seems the std::make_tuple does not behave the same on clang and other
      // compilers.
      // See http://stackoverflow.com/questions/14056000/how-to-avoid-undefined-execution-order-for-the-constructors-when-using-stdmake
      types_.push_back(type);
      values_.push_back(value);
      lengths_.push_back(length);
      formats_.push_back(1 /* binary */);
    #else
      types_.insert(types_.begin(), type);
      values_.insert(values_.begin(), value);
      lengths_.insert(lengths_.begin(), int(length));
      formats_.insert(formats_.begin(), 1 /* binary */);
    #endif
    }

    //--------------------------------------------------------------------------
    // NULL
    //--------------------------------------------------------------------------
    void Params::bind(std::nullptr_t) {
      bind(UNKNOWNOID, (char *)nullptr, 0);
    }

    //--------------------------------------------------------------------------
    // bool
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(bool b) {
      write(b, bind(BOOLOID, sizeof(b)));
    }

    //--------------------------------------------------------------------------
    // smallint
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(int16_t i) {
      write(i, bind(INT2OID, sizeof(i)));
    }

    //--------------------------------------------------------------------------
    // integer
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(int32_t i) {
      write(i, bind(INT4OID, sizeof(i)));
    }

    //--------------------------------------------------------------------------
    // bigint
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(int64_t i) {
      write(i, bind(INT8OID, sizeof(i)));
    }

    //--------------------------------------------------------------------------
    // float
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(float f) {
      write(f, bind(FLOAT4OID, sizeof(f)));
    }

    //--------------------------------------------------------------------------
    // double precision
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(double d) {
      write(d, bind(FLOAT8OID, sizeof(d)));
    }

    //--------------------------------------------------------------------------
    // "char"
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(char c) {
      write(c, bind(CHAROID, sizeof(c)));
    }

    //--------------------------------------------------------------------------
    // varchar
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(const char *sz) {
      if (sz[0] == '\0' && settings_.emptyStringAsNull) {
        bind(nullptr);
      }
      else {
        bind(VARCHAROID, (char *)sz, std::strlen(sz));
      }
    }

    void Params::bind(const std::string &s) {
      if (s.length() == 0 && settings_.emptyStringAsNull) {
        bind(nullptr);
      }
      else {
        bind(VARCHAROID, (char *)s.c_str(), s.length());
      }
    }

    //--------------------------------------------------------------------------
    // bytea
    //--------------------------------------------------------------------------
    void Params::bind(const std::vector<uint8_t> &bytes) {
      bind(BYTEAOID, (char *)bytes.data(), bytes.size());
    }

    //--------------------------------------------------------------------------
    // date
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(date_t d) {
      write(d, bind(DATEOID, sizeof(d)));
    }

    //--------------------------------------------------------------------------
    // timestamptz
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(timestamptz_t d) {
      write(d, bind(TIMESTAMPTZOID, sizeof(d)));
    }

    //--------------------------------------------------------------------------
    // timestamp
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(timestamp_t d) {
      write(d, bind(TIMESTAMPOID, sizeof(d)));
    }

    //--------------------------------------------------------------------------
    // timetz
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(timetz_t t) {
      write(t, bind(TIMETZOID, 12));
    }

    //--------------------------------------------------------------------------
    // time
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(time_t t) {
      write(t, bind(TIMEOID, sizeof(t)));
    }

    //--------------------------------------------------------------------------
    // interval
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(interval_t t) {
      assert(sizeof(t) == 16);
      write(t, bind(INTERVALOID, sizeof(t)));
    }

    //--------------------------------------------------------------------------
    // Arrays
    //--------------------------------------------------------------------------

    template<typename T>
    void Params::bind(Oid arrayType, Oid elemType, const std::vector<array_item<T>> &array) {

      // The value of the array should look like this:
      //
      // struct pg_array {
      //   int32_t ndim; /* Number of dimensions */
      //   int32_t ign;  /* offset for data, removed by libpq */
      //   Oid elemtype; /* type of element in the array */
      //
      //   /* First dimension */
      //   int32_t size;  /* Number of elements */
      //   int32_t index; /* Index of first element */
      //   T first_value; /* Beginning of the data */
      // }

      int32_t bufferSize = 5 * sizeof(int32_t); // array headers
      for (auto &i: array) {
        bufferSize += sizeof(int32_t);
        if (!i.isNull) {
          bufferSize += length(i.value);
        }
      }

      char *buf = bind(arrayType, bufferSize);
      buf = write(int32_t(1), buf);        /* Number of dimensions */
      buf = write(int32_t(0), buf);        /* ignored */
      buf = write(int32_t(elemType), buf); /* type of elements in the array */
      buf = write(int32_t(array.size()), buf); /* Number of elements */
      buf = write(int32_t(1), buf);            /* Index of first element */
      for (auto &i: array) {
        if (i.isNull
            || (elemType == VARCHAROID
                && settings_.emptyStringAsNull
                && length(i.value) == 0)) {
          buf = write(int32_t(-1), buf);
        }
        else {
          buf = write(length(i.value), buf);
          buf = write(i.value, buf);
        }
      }
    }

    template<>
    void Params::bind(const std::vector<array_item<bool>> &array) {
      bind(BOOLARRAYOID, BOOLOID, array);
    }

    template<>
    void Params::bind(const std::vector<array_item<int16_t>> &array) {
      bind(INT2ARRAYOID, INT2OID, array);
    }

    template<>
    void Params::bind(const std::vector<array_item<int32_t>> &array) {
      bind(INT4ARRAYOID, INT4OID, array);
    }

    template<>
    void Params::bind(const std::vector<array_item<int64_t>> &array) {
      bind(INT8ARRAYOID, INT8OID, array);
    }

    template<>
    void Params::bind(const std::vector<array_item<float>> &array) {
      bind(FLOAT4ARRAYOID, FLOAT4OID, array);
    }

    template<>
    void Params::bind(const std::vector<array_item<double>> &array) {
      bind(FLOAT8ARRAYOID, FLOAT8OID, array);
    }

    template<>
    void Params::bind(const std::vector<array_item<std::string>> &array) {
      bind(VARCHARARRAYOID, VARCHAROID, array);
    }

    template<>
    void Params::bind(const std::vector<array_item<date_t>> &array) {
      bind(DATEARRAYOID, DATEOID, array);
    }

    template<>
    void Params::bind(const std::vector<array_item<time_t>> &array) {
      bind(TIMEARRAYOID, TIMEOID, array);
    }

    template<>
    void Params::bind(const std::vector<array_item<timetz_t>> &array) {
      bind(TIMETZARRAYOID, TIMETZOID, array);
    }

    template<>
    void Params::bind(const std::vector<array_item<timestamp_t>> &array) {
      bind(TIMESTAMPARRAYOID, TIMESTAMPOID, array);
    }

    template<>
    void Params::bind(const std::vector<array_item<timestamptz_t>> &array) {
      bind(TIMESTAMPTZARRAYOID, TIMESTAMPTZOID, array);
    }

    template<>
    void Params::bind(const std::vector<array_item<interval_t>> &array) {
      bind(INTERVALARRAYOID, INTERVALOID, array);
    }

  } // namespace postgres
}   // namespace db
