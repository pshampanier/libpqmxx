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

#ifdef __linux__
  #include <arpa/inet.h>
  #include <endian.h>
  #if __BYTE_ORDER == __LITTLE_ENDIAN
    #define htonll(x) __bswap_constant_64(x)
  #else
    #define htonll(x) x
  #endif
#elif defined (__APPLE__)
  #include <arpa/inet.h>
  #include <libkern/OSByteOrder.h>
  #define be16toh(x) OSSwapBigToHostInt16(x)
  #define be32toh(x) OSSwapBigToHostInt32(x)
  #define be64toh(x) OSSwapBigToHostInt64(x)
#elif defined (_WINDOWS)
  #include <winsock2.h>
  #define be16toh(x) ntohs(x)
  #define be32toh(x) ntohl(x)
  #define be64toh(x) ntohll(x)
#endif

namespace db {
  namespace postgres {

    template <>
    int32_t length(timetz_t) {
      return 12;
    };

    int32_t length(const std::string &s) {
      return s.length();
    };

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
      return be16toh(*reinterpret_cast<int16_t*>(move(buf, size)));
    }

    template <>
    char *write(int16_t value, char *buf) {
      *reinterpret_cast<int16_t *>(buf) = htons(value);
      return buf + sizeof(int16_t);
    }

    // -------------------------------------------------------------------------
    // integer
    // -------------------------------------------------------------------------

    template <>
    int32_t read<int32_t>(char **buf, size_t size) {
      return be32toh(*reinterpret_cast<int32_t*>(move(buf, size)));
    }

    template <>
    char *write(int32_t value, char *buf) {
      *reinterpret_cast<int32_t *>(buf) = htonl(value);
      return buf + sizeof(int32_t);
    }

    // -------------------------------------------------------------------------
    // bigint
    // -------------------------------------------------------------------------

    template <>
    int64_t read<int64_t>(char **buf, size_t size) {
      return be64toh(*reinterpret_cast<int64_t*>(move(buf, size)));
    }

    template <>
    char *write(int64_t value, char *buf) {
      *reinterpret_cast<int64_t *>(buf) = htonll(value);
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
