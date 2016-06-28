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
#include "libpq/pg_type.h"

#include <cassert>
#include <cstring>

#include <arpa/inet.h>
#ifdef __linux__
  #include <endian.h>
    #if __BYTE_ORDER == __LITTLE_ENDIAN
      #define htonll(x) __bswap_constant_64(x)
    #else
      #define htonll(x) x
    #endif
#elif defined (__WINDOWS__)

#endif

namespace db {
  namespace postgres {

    //--------------------------------------------------------------------------
    // Constructor
    //--------------------------------------------------------------------------
    Params::Params(int size) {
      types_.reserve(size);
      values_.reserve(size);
      lengths_.reserve(size);
      formats_.reserve(size);
    }

    //--------------------------------------------------------------------------
    // Destructor
    //--------------------------------------------------------------------------
    Params::~Params() {
      for (int i=types_.size(); i > 0; --i) {
        switch (types_[i]) {
          case INT2OID:
          case INT4OID:
          case INT8OID:
          case FLOAT4OID:
          case FLOAT8OID:
          case BOOLOID:
          case CHAROID:
          case DATEOID:
          case TIMESTAMPTZOID:
          case TIMESTAMPOID:
          case TIMETZOID:
          case TIMEOID:
          case INTERVALOID:
            delete [] values_[i];
            break;

          default:
            break;
        }
      }
    }

    //--------------------------------------------------------------------------
    // Bind any value
    //--------------------------------------------------------------------------
    void Params::bind(Oid type, char *value, int length) {

      char *copy = nullptr;
      int format = 1; /* binary */

      switch (type) {
        case INT2OID:
        case INT4OID:
        case INT8OID:
        case FLOAT4OID:
        case FLOAT8OID:
        case BOOLOID:
        case CHAROID:
        case DATEOID:
        case TIMESTAMPTZOID:
        case TIMESTAMPOID:
        case TIMETZOID:
        case TIMEOID:
        case INTERVALOID:
          copy = new char[length];
          std::memcpy(copy, value, length);
          value = copy;
          break;

        case VARCHAROID:
        case BYTEAOID:
        case UNKNOWNOID:
          break;

        default:
          assert(false);
      }

      types_.push_back(type);
      values_.push_back(value);
      lengths_.push_back(length);
      formats_.push_back(format);
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
      bind(BOOLOID, (char *)&b, sizeof(b));
    }

    //--------------------------------------------------------------------------
    // smallint
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(int16_t i) {
      int16_t v = htons(i);
      bind(INT2OID, (char *)&v, sizeof(v));
    }

    //--------------------------------------------------------------------------
    // integer
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(int32_t i) {
      int32_t v = htonl(i);
      bind(INT4OID, (char *)&v, sizeof(v));
    }

    //--------------------------------------------------------------------------
    // bigint
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(int64_t i) {
      int64_t v = htonll(i);
      bind(INT8OID, (char *)&v, sizeof(v));
    }

    //--------------------------------------------------------------------------
    // float
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(float f) {
      uint32_t v = htonl(*reinterpret_cast<uint32_t *>(&f));
      bind(FLOAT4OID, (char *)&v, sizeof(v));
    }

    //--------------------------------------------------------------------------
    // double precision
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(double d) {
      uint64_t v = htonll(*reinterpret_cast<uint64_t *>(&d));
      bind(FLOAT8OID, (char *)&v, sizeof(v));
    }

    //--------------------------------------------------------------------------
    // bool
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(char c) {
      bind(CHAROID, &c, sizeof(c));
    }

    //--------------------------------------------------------------------------
    // varchar
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(const char *sz) {
      bind(VARCHAROID, (char *)sz, std::strlen(sz));
    }

    void Params::bind(const std::string &s) {
      bind(VARCHAROID, (char *)s.c_str(), s.length());
    }

    //--------------------------------------------------------------------------
    // bytea
    //--------------------------------------------------------------------------
    void Params::bind(const std::vector<uint8_t> &d) {
      bind(BYTEAOID, (char *)d.data(), d.size());
    }

    //--------------------------------------------------------------------------
    // date
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(date_t d) {
      int32_t v = ((d.epoch_date - (d.epoch_date % 86400)) / 86400) - DAYS_UNIX_TO_J2000_EPOCH;
      v = htonl(v);
      bind(DATEOID, (char *)&v, sizeof(v));
    }

    //--------------------------------------------------------------------------
    // timestamptz
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(timestamptz_t d) {
      int64_t v = d.epoch_time - MICROSEC_UNIX_TO_J2000_EPOCH;
      v = htonll(v);
      bind(TIMESTAMPTZOID, (char *)&v, sizeof(v));
    }

    //--------------------------------------------------------------------------
    // timestamp
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(timestamp_t d) {
      int64_t v = d.epoch_time - MICROSEC_UNIX_TO_J2000_EPOCH;
      v = htonll(v);
      bind(TIMESTAMPOID, (char *)&v, sizeof(v));
    }

    //--------------------------------------------------------------------------
    // timetz
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(timetz_t t) {
      t.time = htonll(t.time);
      t.offset = htonl(t.offset);
      bind(TIMETZOID, (char *)&t, 12);
    }

    //--------------------------------------------------------------------------
    // time
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(time_t t) {
      int64_t v = htonll(t);
      bind(TIMEOID, (char *)&v, sizeof(v));
    }

    //--------------------------------------------------------------------------
    // timetz
    //--------------------------------------------------------------------------
    template<>
    void Params::bind(interval_t t) {
      assert(sizeof(t) == 16);
      t.months = htonl(t.months);
      t.days = htonl(t.days);
      t.time = htonll(t.time);
      bind(INTERVALOID, (char *)&t, sizeof(t));
    }

  } // namespace postgres
}   // namespace db
