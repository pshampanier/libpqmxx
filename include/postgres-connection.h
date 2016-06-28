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
#include "postgres-params.h"
#include "postgres-result.h"

#include <functional>
#include <memory>
#include <cstddef>

namespace db {
  namespace postgres {

    const int32_t DAYS_UNIX_TO_J2000_EPOCH = int32_t(10957);
    const int64_t MICROSEC_UNIX_TO_J2000_EPOCH = int64_t(946684800) * 1000000;

    typedef struct {
      int32_t epoch_date;
      operator int32_t() const { return epoch_date; }
    } date_t;

    typedef struct {
      int64_t epoch_time;
      operator int64_t() const { return epoch_time; }
    } timestamptz_t;

    typedef struct {
      int64_t epoch_time;
      operator int64_t() const { return epoch_time; }
    } timestamp_t;

    typedef struct {
      int64_t time;
      int32_t offset;
    } timetz_t;

    typedef struct {
      int64_t time;
      operator int64_t() const { return time; }
    } time_t;

    typedef struct {
      int64_t time;
      int32_t days;
      int32_t months;
    } interval_t;

    /**
     * Check if an sql command contains one or more statements.
     *
     * This method does not perform a strict parsing of the SQL. If the `sql`
     * parameter is syntactically incorrect, the return value of this method
     * is undetermined.
     *
     * @param sql The SQL commands to parse.
     * @return true if more than one statement have been found.
     **/
    bool isSingleStatement(const char *sql) noexcept;

    /**
     * A connection to a PostgreSQL database.
     **/
    class Connection : public std::enable_shared_from_this<Connection> {

      friend class Result;

      public:
      
        /**
         * Constructor.
         *
         * Copy and move constructor have been explicitly delete dto prevent copy
         * of the connection object.
         **/
        Connection();

        /**
         * Destructor.
         *
         * The destuctor will implicitly close the connection to the server and
         * associated results if necessary.
         **/
        ~Connection();
      
        /**
         * Open a connection to the database.
         * 
         * @param connInfo A postgresql connection string.
         *
         * Both Keyword/Value and URI are accepted.
         *
         * ```
         *    postgresql://[user[:password]@][netloc][:port][/dbname][?param1=value1&...]
         * ```
         *
         * @see https://www.postgresql.org/docs/9.5/static/libpq-connect.html#LIBPQ-CONNSTRING
         * @return The connection itself.
         **/
        Connection &connect(const char *connInfo);
      
        /**
         * Close the database connection.
         *
         * This method is automatically called by the destructor.
         **/
        void close() noexcept;
      
        /**
         * \defgroup transactions ‘‘Transactions’’
         *
         * Helper methods to deal with transactions. The main benefit of using
         * those methods rather than executing the SQL commands is on nested
         * transactions. If a code start a transaction and call another code
         * also starting a transaction, thoses methods will create only one
         * transaction started at the first all to `begin()` and commited at the
         * last call to `commit()`.
         *
         * @{
         **/

        /**
         * Start a transaction.
         **/
        Connection &begin();

        /**
         * Commit a transaction.
         **/
        Connection &commit();

        /**
         * Rollback a transaction.
         **/
        Connection &rollback();
      
        /* @} */

        /**
         * Cancel queries in progress.
         *
         * Calling this method will request the cancellation of the current query
         * in progress. There is no guaranty the will be cancelled or when it
         * will be cancelled. So usually once you've called this method you can't
         * use anymore the connection since the previous query might still be in
         * progress.
         *
         * @return The connection ifself.
         **/
        Connection &cancel();

        /**
         * Execute one or more SQL commands.
         *
         * @param sql  One or more SQL command to be executed.
         * @param args Zero or more arguments of the SQL command.
         *        If arguments are used, they are referred to in the `sql`
         *        command as `$1`, `$2`, etc... The PostgreSQL datatype of the
         *        argument is deducted from the C++ type of the argument
         *        according to the following table:

           SQL Type                    | C++ Param
           ----------------------------|-----------------------------
           boolean                     | bool
           bytea                       | std::vector\<uint_8\>
           "char"                      | char
           name                        | const char *
           bigint                      | int64_t
           smallint                    | int16_t
           integer                     | int32_t
           text                        | const char *
           real                        | float
           double precision            | double
           character                   | const char *
           character varying           | const char *
           date                        | db::postgres::date_t
           time without time zone      | db::postgres::time_t
           timestamp without time zone | db::postgres::timestamp_t
           timestamp with time zone    | db::postgres::timestamptz_t
           interval                    | db::postgres::interval_t
           time with time zone         | db::postgres::timetz_t

         *
         *
         * @return The result.
         *
         * ```

            // Execute a query with paramters.


            // Execute multiple statements in one call.
            cnx.execute(R"SQL(

              CREATE TYPE GENDER AS ENUM ('M', 'F');

              CREATE TABLE employees (
                emp_no      INTEGER         NOT NULL,
                birth_date  DATE            NOT NULL,
                first_name  VARCHAR(14)     NOT NULL,
                last_name   VARCHAR(16)     NOT NULL,
                gender      GENDER          NOT NULL,
                hire_date   DATE            NOT NULL,
                PRIMARY KEY (emp_no)
              );

              CREATE TABLE titles (
                  emp_no      INT             NOT NULL,
                  title       VARCHAR(50)     NOT NULL,
                  from_date   DATE            NOT NULL,
                  to_date     DATE,
                  FOREIGN KEY (emp_no) REFERENCES employees (emp_no) ON DELETE CASCADE,
                  PRIMARY KEY (emp_no,title, from_date)
              );

            )SQL");

         * ```
         *
         * @attention Arguments are only supported for a single SQL command.
         *
         **/
        template<typename... Args>
        Result &execute(const char *sql, Args... args) {
          Params params(sizeof...(args));
          std::make_tuple((params.bind(std::forward<Args>(args)), 0)...);
          execute(sql, params);
          return result_;
        }

    protected:

        std::string lastError() const;

      private:
        PGconn *pgconn_;
        Result  result_;

        /**
         * Current transaction level.
         *
         * 0 → no transaction in progress.
         * 1 → one transaction in progress.
         * 2 → one transaction in progress + 1 nested transaction.
         * n → one transaction in progress + (n-1) nested transactions.
         **/
        int transaction_;

        void execute(const char *sql, const Params &params);

        operator PGconn *() {
          return pgconn_;
        }

        Connection(const Connection&) = delete;
        Connection(const Connection&&) = delete;
        Connection& operator = (const Connection&) = delete;
        Connection& operator = (const Connection&&) = delete;
    };
    
  } // namespace postgres  
}   // namespace db
