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
#include "postgres-types.h"

#include <cstring>


// -------------------------------------------------------------------------
// Byte-order swapping for 16 bit integers
// -------------------------------------------------------------------------
#define byteorder_swapsc(x)  ((uint16_t)(                        \
                             (((uint16_t)(x) >> 8) & 0x00FF)   | \
                             (((uint16_t)(x) << 8) & 0xFF00)))

// -------------------------------------------------------------------------
// Byte-order swapping for 32 bit integers
// -------------------------------------------------------------------------
#define byteorder_swaplc(x)  ((uint32_t)(                             \
                             (((uint32_t)(x) >> 24) & 0x000000FF)   | \
                             (((uint32_t)(x) >> 8)  & 0x0000FF00)   | \
                             (((uint32_t)(x) << 8)  & 0x00FF0000)   | \
                             (((uint32_t)(x) << 24) & 0xFF000000)))

// -------------------------------------------------------------------------
// Byte-order swapping for 64 bit integers
// -------------------------------------------------------------------------
#define byteorder_swapllc(x) ((uint64_t)(                                   \
                             (((uint64_t)(x) >> 56) & 0x00000000000000FF) | \
                             (((uint64_t)(x) >> 40) & 0x000000000000FF00) | \
                             (((uint64_t)(x) >> 24) & 0x0000000000FF0000) | \
                             (((uint64_t)(x) >> 8)  & 0x00000000FF000000) | \
                             (((uint64_t)(x) << 8)  & 0x000000FF00000000) | \
                             (((uint64_t)(x) << 24) & 0x0000FF0000000000) | \
                             (((uint64_t)(x) << 40) & 0x00FF000000000000) | \
                             (((uint64_t)(x) << 56) & 0xFF00000000000000)))
#ifdef __linux__
  #include <endian.h>
  #if __BYTE_ORDER == __LITTLE_ENDIAN
    #define IS_LITTLE_ENDIAN true
  #else
    #define IS_LITTLE_ENDIAN false
  #endif
#elif defined (__APPLE__)
  #ifdef __LITTLE_ENDIAN__
    #define IS_LITTLE_ENDIAN true
  #else
    #define IS_LITTLE_ENDIAN false
  #endif
#elif defined (WIN32)
  #ifdef __MINGW32__
    #include <sys/param.h>
    #if BYTE_ORDER == LITTLE_ENDIAN
      #define IS_LITTLE_ENDIAN true
    #else
      #define IS_LITTLE_ENDIAN false
    #endif
  #else //MSVC
    #if REG_DWORD == REG_DWORD_LITTLE_ENDIAN
      #define IS_LITTLE_ENDIAN true
    #else
      #define IS_LITTLE_ENDIAN false
    #endif
  #endif
#endif
#ifndef IS_LITTLE_ENDIAN
 #error Cant deternine endianness of host system
#endif

// -------------------------------------------------------------------------
// PostgreSQL stores Integers in network byte order (most significant byte first)
// (https://www.postgresql.org/docs/9.3/static/protocol-overview.html)
// So little-endian systems should swap bytes when transferring them through network
// -------------------------------------------------------------------------
#if IS_LITTLE_ENDIAN
    inline int16_t network_cast(int16_t v){return byteorder_swapsc(v);}
    inline int32_t network_cast(int32_t v){return byteorder_swaplc(v);}
    inline int64_t network_cast(int64_t v){return byteorder_swapllc(v);}
#else
    template <typename T>
    inline T network_cast(T v){return v;}
#endif

namespace db {
  namespace postgres {

    int32_t length(timetz_t) {
      return 12;
    }

    int32_t length(const std::string &s) {
      return int32_t(s.length());
    }

    // -------------------------------------------------------------------------
    // Return the start of a buffer and then move it forward by `size`
    // -------------------------------------------------------------------------
    inline char *move(char **buf, size_t size) {
      char *start = *buf;
      *buf += size;
      return start;
    }


    // -------------------------------------------------------------------------
    // bool
    // -------------------------------------------------------------------------

    template <>
    bool read<bool>(char **buf, size_t size) {
      return *reinterpret_cast<bool*>(move(buf, size));
    }

    template <>
    char *write(bool value, char *buf) {
      *reinterpret_cast<bool *>(buf) = value;
      return buf + sizeof(bool);
    }

    // -------------------------------------------------------------------------
    // smallint
    // -------------------------------------------------------------------------

    template <>
    int16_t read<int16_t>(char **buf, size_t size) {
      return network_cast(*reinterpret_cast<int16_t*>(move(buf, size)));
    }

    template <>
    char *write(int16_t value, char *buf) {
      *reinterpret_cast<int16_t *>(buf) = network_cast(value);
      return buf + sizeof(int16_t);
    }

    // -------------------------------------------------------------------------
    // integer
    // -------------------------------------------------------------------------

    template <>
    int32_t read<int32_t>(char **buf, size_t size) {
      return network_cast(*reinterpret_cast<int32_t*>(move(buf, size)));
    }

    template <>
    char *write(int32_t value, char *buf) {
      *reinterpret_cast<int32_t *>(buf) = network_cast(value);
      return buf + sizeof(int32_t);
    }

    // -------------------------------------------------------------------------
    // bigint
    // -------------------------------------------------------------------------

    template <>
    int64_t read<int64_t>(char **buf, size_t size) {
      return network_cast(*reinterpret_cast<int64_t*>(move(buf, size)));
    }

    template <>
    char *write(int64_t value, char *buf) {
      *reinterpret_cast<int64_t *>(buf) = network_cast(value);
      return buf + sizeof(int64_t);
    }

    // -------------------------------------------------------------------------
    // real
    // -------------------------------------------------------------------------

    template <>
    float read<float>(char **buf, size_t size) {
      int32_t v = read<int32_t>(buf);
      return *reinterpret_cast<float*>(&v);
    }

    template <>
    char *write(float value, char *buf) {
      int32_t v = *reinterpret_cast<int32_t *>(&value);
      return write(v, buf);
    }

    // -------------------------------------------------------------------------
    // double precision
    // -------------------------------------------------------------------------

    template <>
    double read<double>(char **buf, size_t size) {
      int64_t v = read<int64_t>(buf);
      return *reinterpret_cast<double*>(&v);
    }


    template <>
    char *write(double value, char *buf) {
      int64_t v = *reinterpret_cast<int64_t *>(&value);
      return write(v, buf);
    }

    //--------------------------------------------------------------------------
    // "char"
    //--------------------------------------------------------------------------

    template <>
    char *write(char value, char *buf) {
      *buf = value;
      return buf + 1;
    }

    // -------------------------------------------------------------------------
    // char, varchar, text
    // -------------------------------------------------------------------------

    template <>
    std::string read<std::string>(char **buf, size_t size) {
      return std::string(move(buf, size), size);
    }

    template <>
    char *write(const char *value, char *buf) {
      size_t length = std::strlen(value);
      std::memcpy(buf, value, length);
      return buf + length;
    }

    char *write(const std::string &s, char *buf) {
      size_t length = s.length();
      std::memcpy(buf, s.c_str(), length);
      return buf + length;
    }

    // -------------------------------------------------------------------------
    // date
    // -------------------------------------------------------------------------

    template <>
    char *write(date_t d, char *buf) {
      int32_t v = ((d.epoch_date - (d.epoch_date % 86400)) / 86400) - DAYS_UNIX_TO_J2000_EPOCH;
      return write(v, buf);
    }

    template <>
    date_t read<date_t>(char **buf, size_t size) {
      return date_t {
        (read<int32_t>(buf) + DAYS_UNIX_TO_J2000_EPOCH) * 86400
      };
    }

    // -------------------------------------------------------------------------
    // timestamp
    // -------------------------------------------------------------------------

    template <>
    timestamp_t read<timestamp_t>(char **buf, size_t size) {
      return timestamp_t {
        read<int64_t>(buf) + MICROSEC_UNIX_TO_J2000_EPOCH
      };
    }

    template <>
    char *write(timestamp_t d, char *buf) {
      int64_t v = d.epoch_time - MICROSEC_UNIX_TO_J2000_EPOCH;
      return write(v, buf);
    }

    // -------------------------------------------------------------------------
    // timestamptz
    // -------------------------------------------------------------------------

    template <>
    timestamptz_t read<timestamptz_t>(char **buf, size_t size) {
      return timestamptz_t {
        read<int64_t>(buf) + MICROSEC_UNIX_TO_J2000_EPOCH
      };
    }

    template <>
    char *write(timestamptz_t d, char *buf) {
      int64_t v = d.epoch_time - MICROSEC_UNIX_TO_J2000_EPOCH;
      return write(v, buf);
    }

    // -------------------------------------------------------------------------
    // timetz
    // -------------------------------------------------------------------------

    template <>
    timetz_t read<timetz_t>(char **buf, size_t size) {
      return timetz_t {
        read<int64_t>(buf),
        read<int32_t>(buf)
      };
    }

    template <>
    char *write(timetz_t d, char *buf) {
      timetz_t *v = reinterpret_cast<timetz_t *>(buf);
      write(d.time, reinterpret_cast<char *>(&v->time));
      write(d.offset, reinterpret_cast<char *>(&v->offset));
      return buf + length(d);
    }

    // -------------------------------------------------------------------------
    // time
    // -------------------------------------------------------------------------

    template <>
    time_t read<time_t>(char **buf, size_t size) {
      return time_t { read<int64_t>(buf) };
    }

    template <>
    char *write(time_t t, char *buf) {
      return write(t.time, buf);
    }

    // -------------------------------------------------------------------------
    // interval
    // -------------------------------------------------------------------------

    template <>
    interval_t read<interval_t>(char **buf, size_t size) {
      return interval_t {
        read<int64_t>(buf),
        read<int32_t>(buf),
        read<int32_t>(buf)
      };
    }

    template <>
    char *write(interval_t t, char *buf) {
      interval_t *v = reinterpret_cast<interval_t *>(buf);
      write(t.months, reinterpret_cast<char *>(&v->months));
      write(t.days, reinterpret_cast<char *>(&v->days));
      write(t.time, reinterpret_cast<char *>(&v->time));
      return buf + sizeof(interval_t);
    }

  } // namespace postgres
}   // namespace db
