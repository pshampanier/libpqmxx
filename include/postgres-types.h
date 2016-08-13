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

#include "libpq-fe.h"

#include <cstddef>
#include <string>
#include <vector>
#include <stdint.h>

#define 	BOOLOID   16
#define 	BYTEAOID   17
#define 	CHAROID   18
#define 	NAMEOID   19
#define 	INT8OID   20
#define 	INT2OID   21
#define 	INT2VECTOROID   22
#define 	INT4OID   23
#define 	REGPROCOID   24
#define 	TEXTOID   25
#define 	OIDOID   26
#define 	TIDOID   27
#define 	XIDOID   28
#define 	CIDOID   29
#define 	OIDVECTOROID   30
#define 	JSONOID   114
#define 	XMLOID   142
#define 	PGNODETREEOID   194
#define   JSONARRAYOID 199
#define 	PGDDLCOMMANDOID   32
#define 	POINTOID   600
#define 	LSEGOID   601
#define 	PATHOID   602
#define 	BOXOID   603
#define 	POLYGONOID   604
#define 	LINEOID   628
#define 	FLOAT4OID   700
#define 	FLOAT8OID   701
#define 	ABSTIMEOID   702
#define 	RELTIMEOID   703
#define 	TINTERVALOID   704
#define 	UNKNOWNOID   705
#define 	CIRCLEOID   718
#define 	CASHOID   790
#define   CASHARRAYOID 791
#define 	MACADDROID   829
#define 	INETOID   869
#define 	CIDROID   650
#define   BOOLARRAYOID 1000
#define   BYTEAARRAYOID 1001
#define   CHARARRAYOID 1002
#define 	INT2ARRAYOID   1005
#define 	INT4ARRAYOID   1007
#define 	TEXTARRAYOID   1009
#define   XIDARRAYOID 1011
#define   CIDARRAYOID 1012
#define   BPCHARARRAYOID 1014
#define   VARCHARARRAYOID 1015
#define   INT8ARRAYOID 1016
#define   FLOAT8ARRAYOID 1022
#define   ABSTIMEARRAYOID 1023
#define   RELTIMEARRAYOID 1024
#define   TIMESTAMPARRAYOID 1115
#define   DATEARRAYOID 1182
#define   TIMEARRAYOID 1183
#define   TIMESTAMPTZARRAYOID 1185
#define   INTERVALARRAYOID 1187
#define   NUMERICARRAYOID 1231
#define   TIMETZARRAYOID 1270
#define 	OIDARRAYOID   1028
#define 	FLOAT4ARRAYOID   1021
#define 	ACLITEMOID   1033
#define 	CSTRINGARRAYOID   1263
#define 	BPCHAROID   1042
#define 	VARCHAROID   1043
#define 	DATEOID   1082
#define 	TIMEOID   1083
#define 	TIMESTAMPOID   1114
#define 	TIMESTAMPTZOID   1184
#define 	INTERVALOID   1186
#define 	TIMETZOID   1266
#define 	BITOID   1560
#define 	VARBITOID   1562
#define 	NUMERICOID   1700
#define 	REFCURSOROID   1790
#define 	REGPROCEDUREOID   2202
#define 	REGOPEROID   2203
#define 	REGOPERATOROID   2204
#define 	REGCLASSOID   2205
#define 	REGTYPEOID   2206
#define 	REGROLEOID   4096
#define 	REGNAMESPACEOID   4089
#define 	REGTYPEARRAYOID   2211
#define 	UUIDOID   2950
#define 	LSNOID   3220
#define 	TSVECTOROID   3614
#define 	GTSVECTOROID   3642
#define 	TSQUERYOID   3615
#define 	REGCONFIGOID   3734
#define 	REGDICTIONARYOID   3769
#define 	JSONBOID   3802
#define 	INT4RANGEOID   3904
#define 	RECORDOID   2249
#define 	RECORDARRAYOID   2287
#define 	CSTRINGOID   2275
#define 	ANYOID   2276
#define 	ANYARRAYOID   2277
#define 	VOIDOID   2278
#define 	TRIGGEROID   2279
#define 	EVTTRIGGEROID   3838
#define 	LANGUAGE_HANDLEROID   2280
#define 	INTERNALOID   2281
#define 	OPAQUEOID   2282
#define 	ANYELEMENTOID   2283
#define 	ANYNONARRAYOID   2776
#define 	ANYENUMOID   3500
#define 	FDW_HANDLEROID   3115
#define 	INDEX_AM_HANDLEROID   325
#define 	TSM_HANDLEROID   3310
#define   JSONBARRAYOID 3807
#define 	ANYRANGEOID   3831

namespace db {
  namespace postgres {

    /**
     * A `date` value.
     *
     * This struct can be used set a date parameter when calling execute, or to
     * get a date value from a Row. An alternative for date parameters is to use
     * a date literal with an explict cast of the parameter in the sql command.
     *
     * ```
     * execute("SELECT $1::date", "2014-11-01");
     * ```
     **/
    typedef struct {
      int32_t epoch_date; /**< Number of seconds sine Unix epoch time. **/
      operator int32_t() const { return epoch_date; } /**< Cast to `int32_t`. **/
    } date_t;

    /**
     * A `timestamp with timezone` value.
     *
     * This struct can be used set a timestamptz parameter when calling execute,
     * or to get a timestamptz value from a Row. An alternative for timestamptz
     * parameters is to use a timestamp literal with an explict cast of the
     * parameter in the sql command.
     *
     * ```
     * execute("SELECT $1::timestamptz", "2014-11-01T05:19:00-500");
     * ```
     **/
    typedef struct {
      int64_t epoch_time; /**< Number of microsecondes since Unix epoch time. **/
      operator int64_t() const { return epoch_time; } /**< Cast to int64_t. **/
    } timestamptz_t;

    /**
     * A `timestamp` value (without time zone).
     *
     * This struct can be used set a timestamp parameter when calling execute,
     * or to get a timestamp value from a Row. An alternative for timestamp
     * parameters is to use a timestamp literal with an explict cast of the
     * parameter in the sql command.
     *
     * ```
     * execute("SELECT $1::timestamp", "2014-11-01T05:19:00");
     * ```
     **/
    typedef struct {
      int64_t epoch_time; /**< Number of microsecondes since Unix epoch time. **/
      operator int64_t() const { return epoch_time; }   /**< Cast to int64_t. **/
    } timestamp_t;

    /**
     * A `time with timezone` value.
     **/
    typedef struct {
      int64_t time;   /**< Number of microseconds since 00:00:00. **/
      int32_t offset; /**< Offset from GMT in seconds. **/
    } timetz_t;

    /**
     * A `time` value.
     **/
    typedef struct {
      int64_t time;              /**< Number of microseconds since 00:00:00. **/
      operator int64_t() const { return time; }        /**< Cast to int64_t. **/
    } time_t;

    /**
     * An `interval` value.
     **/
    typedef struct {
      int64_t time;   /**< Number of microseconds on the day since 00:00:00. **/
      int32_t days;   /**< Number of days. **/
      int32_t months; /**< Number of months. **/
    } interval_t;

    /**
     * A values in an array.
     **/
    template <typename T>
    struct array_item {
      T     value;
      bool  isNull;

      array_item() = default;

      array_item(T v) {
        value = v;
        isNull = false;
      }

      /**
       * null value in an array.
       **/
      array_item(std::nullptr_t) {
        isNull = true;
      }

      bool operator==(const array_item<T> &other) const {
        if (isNull) {
          return other.isNull;
        }
        else {
          return other.value == value;
        }
      }

    };

    typedef std::vector<array_item<bool>>          array_bool_t;        /**< Array of `boolean` values. **/
    typedef std::vector<array_item<char>>          array_char_t;        /**< Array of `"char"` values. **/
    typedef std::vector<array_item<int16_t>>       array_int16_t;       /**< Array of `smallint` values. **/
    typedef std::vector<array_item<int32_t>>       array_int32_t;       /**< Array of `integer` values. **/
    typedef std::vector<array_item<int64_t>>       array_int64_t;       /**< Array of `bigint` values. **/
    typedef std::vector<array_item<float>>         array_float_t;       /**< Array of `real` values. **/
    typedef std::vector<array_item<double>>        array_double_t;      /**< Array of `double precision` values. **/
    typedef std::vector<array_item<std::string>>   array_string_t;      /**< Array of `character varying` values. **/
    typedef std::vector<array_item<date_t>>        array_date_t;        /**< Array of `date` values. **/
    typedef std::vector<array_item<time_t>>        array_time_t;        /**< Array of `time without time zone` values. **/
    typedef std::vector<array_item<timetz_t>>      array_timetz_t;      /**< Array of `time with time zone` values. **/
    typedef std::vector<array_item<timestamp_t>>   array_timestamp_t;   /**< Array of `timestamp without time zone` values. **/
    typedef std::vector<array_item<timestamptz_t>> array_timestamptz_t; /**< Array of `timestamp with time zone ` values. **/
    typedef std::vector<array_item<interval_t>>    array_interval_t;    /**< Array of `interval` values. **/

    const int32_t DAYS_UNIX_TO_J2000_EPOCH = int32_t(10957);
    const int64_t MICROSEC_UNIX_TO_J2000_EPOCH = int64_t(946684800) * 1000000;

    /**
     * Reading a value from a postgresql value buffer.
     *
     * @param buf A pointer to a buffer containing a postgresql value.
     *        The position in the buffer is moved forward by the given `size`.
     * @param size Number of bytes of the value.
     * @return The value read from the buffer.
     **/
    template <typename T>
    T read(char **buf, size_t size = sizeof(T));

    template <typename T>
    char *write(T value, char *buf);

    char *write(const std::string &value, char *buf);

    template <typename T>
    int32_t length(T v) {
      return sizeof(v);
    };

    int32_t length(const std::string &value);

  } // namespace postgres
}   // namespace db
