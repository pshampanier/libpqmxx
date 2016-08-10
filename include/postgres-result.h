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

namespace db {
  namespace postgres {
    
    class Connection;
    class Result;

    /**
     * A row in a Result.
     *
     * Rows can be accessed using the Result::iterator except the first one
     * that can be directly access through the result.
     **/
    class Row {

      friend class Result;

    public:

      /**
       * Test a column for a null value.
       *
       * @param column Column number. Column numbers start at 0.
       * @return true if the column value is a null value.
       **/
      bool isNull(int column) const;

      /**
       * Get a column value.
       *
       * ```
       * int32_t emp_no = row.as<int32_t>(0);
       * ```
       *
       * The `typename` used to call the function must by compatible with the
       * SQL type. The following table defines compatible types:

         SQL Type                    | Typename                    | null value
         ----------------------------|-----------------------------|------------
         boolean                     | bool                        | false
         bytea                       | std::vector\<uint_8\>       | *empty vector*
         "char"                      | char                        | '\0'
         name                        | std::string                 | *empty string*
         bigint                      | int64_t                     | 0
         smallint                    | int16_t                     | 0
         integer                     | int32_t                     | 0
         text                        | std::string                 | *empty string*
         real                        | float                       | 0.f
         double precision            | double                      | 0.
         character                   | std::string                 | *empty string*
         character varying           | std::string                 | *empty string*
         date                        | db::postgres::date_t        | { 0 }
         time without time zone      | db::postgres::time_t        | { 0 }
         timestamp without time zone | db::postgres::timestamp_t   | { 0 }
         timestamp with time zone    | db::postgres::timestamptz_t | { 0 }
         interval                    | db::postgres::interval_t    | { 0, 0 }
         time with time zone         | db::postgres::timetz_t      | { 0, 0, 0 }
         smallserial                 | int16_t                     | 0
         serial                      | int32_t                     | 0
         bigserial                   | int64_t                     | 0

       * If the column value is null, the null value defined in the table above
       * will be returned. To insure the column value is really null the method
       * isNull() should be used.
       *
       * @param column Column number. Column numbers start at 0.
       * @return The value of the column.
       *
       * @attention Calling this method with incompatible types is treated as
       *            programming errors, not user or run-time errors. Those errors
       *            will be captured by an assert in debug mode and the behavior
       *            in non debug modes is undertermined.
       **/
      template<typename T>
      T as(int column) const;

      /**
       * Get a column values for arrays.
       *

        ```
        array_int32_t quarters = row.asArray<int32_t>(0);
        ```

       *
       * Usage is the similar to using `T as(int column)` and binding between
       * SQL names and C++ types are the same. Only bytea is not
       * supported.
       *
       * Only array of one dimention are supported.
       *
       * @param column Column number. Column numbers start at 0.
       * @return The value of the column. The value is actually a vector
       *         of array_item<T>. Each item has two properties (`value` and
       *         `isNull`).
       **/
      template<typename T>
      std::vector<array_item<T>> asArray(int column) const;

      /**
       * Get a column name.
       *
       * @param column Column number. Column numbers start at 0.
       * @return The column name associated with the given column number.
       *

        ```
        cnx.execute("SELECT emp_no, firstname from employees").columnName(1) // → "firstname"
        ```

       *
       **/
      const char *columnName(int column) const;

      /**
       * Get the row number.
       *
       * @return For each row returned by a query, num() returns a number
       *         indicating the order of the row in the result. The first row
       *         selected has a num() of 1, the second has 2, and so on.
       **/
      int num() const noexcept;

      /**
       * Get a column value.
       *
       * @deprecated use Row::as(int column) for replacement.
       **/
      template<typename T>
      T get(int column) const {
        return as<T>(column);
      }

    private:
      Result &result_;  /**< The result set by the constructor **/

      /**
       * Constructor.
       **/
      Row(Result &result);

      Row(const Row&) = delete;
      Row& operator = (const Row&) = delete;
      Row(const Row&&) = delete;
      Row& operator = (const Row&&) = delete;
    };

    /**
     * A result from an SQL command.
     *
     * SQL commands always return a result. For a SELECT command you can iterate
     * through the result using the Result::iterator. Otherwise you can use the
     * count() method to get the number of rows affected by the SQL command.
     *
     * When executing more than one SQL command, the iterator can also be used
     * to access the result of each command.
     **/
    class Result : public Row {

      friend class Connection;
      friend class Row;

    public:
      
      /**
       * Number of rows affected by the SQL command.
       *
       * This function can only be used following the execution of a SELECT,
       * CREATE TABLE AS, INSERT, UPDATE, DELETE, MOVE, FETCH, or COPY statement,
       * or an EXECUTE of a prepared query that contains an INSERT, UPDATE, or
       * DELETE statement. If the command that generated the PGresult was
       * anything else, count() returns 0.
       *
       * @return The number of rows affected by the SQL statement.
       **/
      uint64_t count() const noexcept;
      
      /**
       * Support of the range-based for loops.
       *
       *
       * ```
       *  auto &result = cnx.exectute("SELECT ...");
       *  for (auto &row: result) {
       *    ...
       *  }
       * ```
       *
       **/
      class iterator {
      public:

        iterator(Row *ptr): ptr_(ptr) {} /**< Constructor. **/
        iterator operator ++();          /**< Next row in the resultset **/
        bool operator != (const iterator &other) { return ptr_ != other.ptr_; }
        Row &operator *() { return *ptr_; }

      private:
        Row *ptr_;
      };

      /**
       * First row of the result.
       *
       * @return An iterator pointing to the first row of the result.
       **/
      iterator begin();

      /**
       * Last row of the result.
       *
       * @return An iterator pointing to the past-the-end row of the result.
       **/
      iterator end();

    private:

      PGresult *pgresult_;  /**< Native result **/
      Connection &conn_;    /**< Connection owning the result. **/
      Row begin_, end_;     /**< Virtual begin and end of the result. **/
      int num_;             /**< Current row number. */

      ExecStatusType status_ = PGRES_EMPTY_QUERY;

      Result(Connection &conn);
      ~Result();

      /**
       * Cast to the native PostgreSQL result.
       **/
      operator const PGresult *() const {
        return pgresult_;
      }

      /**
       * Get the first result from the server.
       **/
      void first();

      /**
       * Get the next result from the server.
       **/
      void next();

      /**
       * Clear the previous result of the connection.
       **/
      void clear();

      Result(const Result&) = delete;
      Result(const Result&&) = delete;
      Result& operator = (const Result&) = delete;
      Result& operator = (const Result&&) = delete;
    };
    
  } // namespace postgres
}   // namespace db
