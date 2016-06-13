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

namespace db {
  namespace postgres {
    
    class Connection;
    class Result;

    class Row {

      friend class Result;

    public:
      Row(Result &result);

      template<typename T>
      T get(int column) const;

      /**
       * Row number.
       *
       * @return For each row returned by a query, `num()` returns a number
       *         indicating the order of the row in the result. The first row
       *         selected has a `num()` of 1, the second has 2, and so on.
       **/
      int num() const noexcept;

    private:
      Result &result_;

      Row(const Row&) = delete;
      Row& operator = (const Row&) = delete;
      Row(const Row&&) = delete;
      Row& operator = (const Row&&) = delete;
    };

    class Result : public Row {

      friend class Connection;
      friend class Row;

    public:

      Result(Connection &conn);
      ~Result();
      
      /**
       * Number of rows affected by the SQL command.
       *
       * This function can only be used following the execution of a SELECT,
       * CREATE TABLE AS, INSERT, UPDATE, DELETE, MOVE, FETCH, or COPY statement,
       * or an EXECUTE of a prepared query that contains an INSERT, UPDATE, or
       * DELETE statement. If the command that generated the PGresult was
       * anything else, `count()` returns 0.
       *
       * @return The number of rows affected by the SQL statement.
       **/
      uint64_t count() const noexcept;
      
      /**
       * iterator - Support of the range-based for loops.
       *
       *
       * ```
       *  for (auto &row: result) {
       *    ...
       *  }
       * ```
       *
       **/
      class iterator {
      public:

        iterator(Row *ptr): ptr_(ptr) {}
        iterator operator ++();
        bool operator != (const iterator &other) { return ptr_ != other.ptr_; }
        Row &operator *() { return *ptr_; }

      private:
        Row *ptr_;
      };

      iterator begin();
      iterator end();

    private:

      /**
       * The current result.
       **/
      PGresult *pgresult_;

      Connection &conn_;
      Row begin_, end_;

      int num_;

      ExecStatusType status_ = PGRES_EMPTY_QUERY;

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
