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

#include <cstddef>
#include <string>
#include <vector>
#include <stdint.h>


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


  } // namespace postgres
}   // namespace db
