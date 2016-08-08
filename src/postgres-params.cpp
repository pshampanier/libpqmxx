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

#ifdef __linux__
  #include <arpa/inet.h>
  #include <endian.h>
    #if __BYTE_ORDER == __LITTLE_ENDIAN
      #define htonll(x) __bswap_constant_64(x)
    #else
      #define htonll(x) x
    #endif
#elif defined (_WINDOWS)
  #include <winsock2.h>
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

    template <typename T>
    void write(T value, char *buf);

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

    template <>
    void write(bool value, char *buf) {
      *reinterpret_cast<bool *>(buf) = value;
    }

    template<>
    void Params::bind(bool b) {
      write(b, bind(BOOLOID, sizeof(b)));
    }

    //--------------------------------------------------------------------------
    // smallint
    //--------------------------------------------------------------------------

    template <>
    void write(int16_t value, char *buf) {
      *reinterpret_cast<int16_t *>(buf) = htons(value);
    }

    template<>
    void Params::bind(int16_t i) {
      write(i, bind(INT2OID, sizeof(i)));
    }

    //--------------------------------------------------------------------------
    // integer
    //--------------------------------------------------------------------------

    template <>
    void write(int32_t value, char *buf) {
      *reinterpret_cast<int32_t *>(buf) = htonl(value);
    }

    template<>
    void Params::bind(int32_t i) {
      write(i, bind(INT4OID, sizeof(i)));
    }

    //--------------------------------------------------------------------------
    // bigint
    //--------------------------------------------------------------------------

    template <>
    void write(int64_t value, char *buf) {
      *reinterpret_cast<int64_t *>(buf) = htonll(value);
    }

    template<>
    void Params::bind(int64_t i) {
      write(i, bind(INT8OID, sizeof(i)));
    }

    //--------------------------------------------------------------------------
    // float
    //--------------------------------------------------------------------------

    template <>
    void write(float value, char *buf) {
      int32_t v = *reinterpret_cast<int32_t *>(&value);
      write(v, buf);
    }

    template<>
    void Params::bind(float f) {
      write(f, bind(FLOAT4OID, sizeof(f)));
    }

    //--------------------------------------------------------------------------
    // double precision
    //--------------------------------------------------------------------------

    template <>
    void write(double value, char *buf) {
      int64_t v = *reinterpret_cast<int64_t *>(&value);
      write(v, buf);
    }

    template<>
    void Params::bind(double d) {
      write(d, bind(FLOAT8OID, sizeof(d)));
    }

    //--------------------------------------------------------------------------
    // "char"
    //--------------------------------------------------------------------------

    template <>
    void write(char value, char *buf) {
      *buf = value;
    }

    template<>
    void Params::bind(char c) {
      write(c, bind(CHAROID, sizeof(c)));
    }

    //--------------------------------------------------------------------------
    // varchar
    //--------------------------------------------------------------------------

    template <>
    void write(const char *value, char *buf) {
      std::memcpy(buf, value, std::strlen(value));
    }

    template <>
    void write(const std::string &value, char *buf) {
      std::memcpy(buf, value.c_str(), value.length());
    }

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

    template <>
    void write(date_t d, char *buf) {
      int32_t v = ((d.epoch_date - (d.epoch_date % 86400)) / 86400) - DAYS_UNIX_TO_J2000_EPOCH;
      write(v, buf);
    }

    template<>
    void Params::bind(date_t d) {
      write(d, bind(DATEOID, sizeof(d)));
    }

    //--------------------------------------------------------------------------
    // timestamptz
    //--------------------------------------------------------------------------

    template <>
    void write(timestamptz_t d, char *buf) {
      int64_t v = d.epoch_time - MICROSEC_UNIX_TO_J2000_EPOCH;
      write(v, buf);
    }

    template<>
    void Params::bind(timestamptz_t d) {
      write(d, bind(TIMESTAMPTZOID, sizeof(d)));
    }

    //--------------------------------------------------------------------------
    // timestamp
    //--------------------------------------------------------------------------

    void write(timestamp_t d, char *buf) {
      int64_t v = d.epoch_time - MICROSEC_UNIX_TO_J2000_EPOCH;
      write(v, buf);
    }

    template<>
    void Params::bind(timestamp_t d) {
      write(d, bind(TIMESTAMPOID, sizeof(d)));
    }

    //--------------------------------------------------------------------------
    // timetz
    //--------------------------------------------------------------------------

    void write(timetz_t d, char *buf) {
      timetz_t *v = reinterpret_cast<timetz_t *>(buf);
      write(d.time, reinterpret_cast<char *>(&v->time));
      write(d.offset, reinterpret_cast<char *>(&v->offset));
    }

    template<>
    void Params::bind(timetz_t t) {
      write(t, bind(TIMETZOID, 12));
    }

    //--------------------------------------------------------------------------
    // time
    //--------------------------------------------------------------------------

    void write(time_t t, char *buf) {
      write(t.time, buf);
    }

    template<>
    void Params::bind(time_t t) {
      write(t, bind(TIMEOID, sizeof(t)));
    }

    //--------------------------------------------------------------------------
    // timetz
    //--------------------------------------------------------------------------

    void write(interval_t t, char *buf) {
      interval_t *v = reinterpret_cast<interval_t *>(buf);
      write(t.months, reinterpret_cast<char *>(&v->months));
      write(t.days, reinterpret_cast<char *>(&v->days));
      write(t.time, reinterpret_cast<char *>(&v->time));
    }

    template<>
    void Params::bind(interval_t t) {
      assert(sizeof(t) == 16);
      write(t, bind(INTERVALOID, sizeof(t)));
    }

    //--------------------------------------------------------------------------
    // Arrays
    //--------------------------------------------------------------------------

    template<typename T>
    void Params::bind(Oid itemsType, std::vector<T> value) {


    }

    template<>
    void Params::bind(std::vector<int16_t> values) {
      return;
    };

  } // namespace postgres
}   // namespace db
