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

namespace db {
  namespace postgres {

    const Oid BOOLOID = 16;
    const Oid BYTEAOID = 17;
    const Oid CHAROID = 18;
    const Oid NAMEOID = 19;
    const Oid INT8OID = 20;
    const Oid INT2OID = 21;
    const Oid INT2VECTOROID = 22;
    const Oid INT4OID = 23;
    const Oid REGPROCOID = 24;
    const Oid TEXTOID = 25;
    const Oid OIDOID = 26;
    const Oid TIDOID = 27;
    const Oid XIDOID = 28;
    const Oid CIDOID = 29;
    const Oid OIDVECTOROID = 30;
    const Oid JSONOID = 114;
    const Oid XMLOID = 142;
    const Oid PGNODETREEOID = 194;
    const Oid JSONARRAYOID = 199;
    const Oid PGDDLCOMMANDOID = 32;
    const Oid POINTOID = 600;
    const Oid LSEGOID = 601;
    const Oid PATHOID = 602;
    const Oid BOXOID = 603;
    const Oid POLYGONOID = 604;
    const Oid LINEOID = 628;
    const Oid FLOAT4OID = 700;
    const Oid FLOAT8OID = 701;
    const Oid ABSTIMEOID = 702;
    const Oid RELTIMEOID = 703;
    const Oid TINTERVALOID = 704;
    const Oid UNKNOWNOID = 705;
    const Oid CIRCLEOID = 718;
    const Oid CASHOID = 790;
    const Oid CASHARRAYOID = 791;
    const Oid MACADDROID = 829;
    const Oid INETOID = 869;
    const Oid CIDROID = 650;
    const Oid BOOLARRAYOID = 1000;
    const Oid BYTEAARRAYOID = 1001;
    const Oid CHARARRAYOID = 1002;
    const Oid INT2ARRAYOID = 1005;
    const Oid INT4ARRAYOID = 1007;
    const Oid TEXTARRAYOID = 1009;
    const Oid XIDARRAYOID = 1011;
    const Oid CIDARRAYOID = 1012;
    const Oid BPCHARARRAYOID = 1014;
    const Oid VARCHARARRAYOID = 1015;
    const Oid INT8ARRAYOID = 1016;
    const Oid FLOAT8ARRAYOID = 1022;
    const Oid ABSTIMEARRAYOID = 1023;
    const Oid RELTIMEARRAYOID = 1024;
    const Oid TIMESTAMPARRAYOID = 1115;
    const Oid DATEARRAYOID = 1182;
    const Oid TIMEARRAYOID = 1183;
    const Oid TIMESTAMPTZARRAYOID = 1185;
    const Oid INTERVALARRAYOID = 1187;
    const Oid NUMERICARRAYOID = 1231;
    const Oid TIMETZARRAYOID = 1270;
    const Oid OIDARRAYOID = 1028;
    const Oid FLOAT4ARRAYOID = 1021;
    const Oid ACLITEMOID = 1033;
    const Oid CSTRINGARRAYOID = 1263;
    const Oid BPCHAROID = 1042;
    const Oid VARCHAROID = 1043;
    const Oid DATEOID = 1082;
    const Oid TIMEOID = 1083;
    const Oid TIMESTAMPOID = 1114;
    const Oid TIMESTAMPTZOID = 1184;
    const Oid INTERVALOID = 1186;
    const Oid TIMETZOID = 1266;
    const Oid BITOID = 1560;
    const Oid VARBITOID = 1562;
    const Oid NUMERICOID = 1700;
    const Oid REFCURSOROID = 1790;
    const Oid REGPROCEDUREOID = 2202;
    const Oid REGOPEROID = 2203;
    const Oid REGOPERATOROID = 2204;
    const Oid REGCLASSOID = 2205;
    const Oid REGTYPEOID = 2206;
    const Oid REGROLEOID = 4096;
    const Oid REGNAMESPACEOID = 4089;
    const Oid REGTYPEARRAYOID = 2211;
    const Oid UUIDOID = 2950;
    const Oid LSNOID = 3220;
    const Oid TSVECTOROID = 3614;
    const Oid GTSVECTOROID = 3642;
    const Oid TSQUERYOID = 3615;
    const Oid REGCONFIGOID = 3734;
    const Oid REGDICTIONARYOID = 3769;
    const Oid JSONBOID = 3802;
    const Oid INT4RANGEOID = 3904;
    const Oid RECORDOID = 2249;
    const Oid RECORDARRAYOID = 2287;
    const Oid CSTRINGOID = 2275;
    const Oid ANYOID = 2276;
    const Oid ANYARRAYOID = 2277;
    const Oid VOIDOID = 2278;
    const Oid TRIGGEROID = 2279;
    const Oid EVTTRIGGEROID = 3838;
    const Oid LANGUAGE_HANDLEROID = 2280;
    const Oid INTERNALOID = 2281;
    const Oid OPAQUEOID = 2282;
    const Oid ANYELEMENTOID = 2283;
    const Oid ANYNONARRAYOID = 2776;
    const Oid ANYENUMOID = 3500;
    const Oid FDW_HANDLEROID = 3115;
    const Oid INDEX_AM_HANDLEROID = 325;
    const Oid TSM_HANDLEROID = 3310;
    const Oid JSONBARRAYOID = 3807;
    const Oid ANYRANGEOID = 3831;

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
      T     value;    /**< The value for non null values. **/
      bool  isNull;   /**< `true` if the value is null. **/

      array_item() = default;

      /** Constructor of a non null value.
        *
        * @param v  The value.
        **/
      array_item(T v) {
        value = v;
        isNull = false;
      }

      /**
       * Constructor of a null value.
       **/
      array_item(std::nullptr_t) {
        isNull = true;
      }

      operator const T &() const {
        return value;
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

    /**
     * Writing a value to a postgresql buffer.
     *
     * @param value The value to write.
     * @param buf   The start of the PostgreSQL buffer.
     * @return The position next to the end of the value in the PostreSQL buffer `buf`.
     **/
    template <typename T>
    char *write(T value, char *buf);
    char *write(const std::string &value, char *buf);

    /**
     * Length of a value in PostgreSQL buffer.
     *
     * @param value A value of any type.
     * @return The number of bytes the value will take in a PostgreSQL buffer.
     **/
    template <typename T>
    int32_t length(T value) {
      return sizeof(value);
    };
    int32_t length(const std::string &value);
    int32_t length(timetz_t value);

  } // namespace postgres
}   // namespace db
