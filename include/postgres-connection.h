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

#include "postgres-params.h"
#include "postgres-result.h"

#include <functional>
#include <memory>
#include <cstddef>

namespace db {
  namespace postgres {

    /**
     * Settings of a PostgreSQL connection.
     *
     * Those settings are internal setting of the libpqmxx library. Other
     * standard PostgreSQL seetings should be set when calling connect().
     **/
    struct Settings {
      /**
       * If true (the default), empty strings passed as parameter to execute() are
       * considered as null values.
       **/
      bool emptyStringAsNull = true;
    };

    /**
     * A connection to a PostgreSQL database.
     **/
    class Connection : public std::enable_shared_from_this<Connection> {

      friend class Result;

      public:
      
        /**
         * Constructor.
         *
         * Copy and move constructor have been explicitly deleted to prevent the
         * copy of the connection object.
         **/
        Connection(Settings settings = Settings());

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
         *                 Both Keyword/Value and URI are accepted.
         *                 The passed `connInfo` can be empty or null to use all
         *                 default parameters
         *                 (see [Environment Variables](https://www.postgresql.org/docs/9.5/static/libpq-envars.html)).
         *
         * ```
         * postgresql://[user[:password]@][netloc][:port][/dbname][?param1=value1&...]
         * ```
         *
         * @see https://www.postgresql.org/docs/9.5/static/libpq-connect.html#LIBPQ-CONNSTRING
         * @return The connection itself.
         **/
        Connection &connect(const char *connInfo = nullptr);
      
        /**
         * Close the database connection.
         *
         * This method is automatically called by the destructor.
         **/
        Connection &close() noexcept;

        /**
         * Cancel queries in progress.
         *
         * Calling this method will request the cancellation of the current query
         * in progress. There is no guaranty the query will be cancelled or when
         * it will be cancelled. So usually once you've called this method you
         * can't use anymore the connection since the previous query might still
         * be in progress.
         *
         * @return The connection ifself.
         **/
        Connection &cancel();

        /**
         * Execute one or more SQL commands.
         *
         * @param sql  One or more SQL commands to be executed.
         * @param args Zero or more parameters of the SQL command.
         *        If parameters are used, they are referred to in the `sql`
         *        command as `$1`, `$2`, etc... The PostgreSQL datatype of a
         *        parameter is deducted from the C++ type of the parameter
         *        according to the following table:

           SQL Type                    | C++ Param
           ----------------------------|-----------------------------
           boolean                     | bool
           bytea                       | std::vector\<uint_8\>
           "char"                      | char
           bigint                      | int64_t
           smallint                    | int16_t
           integer                     | int32_t
           real                        | float
           double precision            | double
           character varying           | const char *, std::string
           date                        | db::postgres::date_t
           time without time zone      | db::postgres::time_t
           timestamp without time zone | db::postgres::timestamp_t
           timestamp with time zone    | db::postgres::timestamptz_t
           interval                    | db::postgres::interval_t
           time with time zone         | db::postgres::timetz_t

         *
         * ```

          // Execute a query
          auto results = cnx.execute("SELECT MAX(emp_no) FROM employees");

          // Execute a query with parameters.
          cnx.execute("UPDATE titles SET to_date=$1::date WHERE emp_no=$2", "1988-02-10", 10020);

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

          )SQL");

         * ```
         * When the parameter data type might be ambigious for the server, you
         * can use the PostgreSQL casting syntax. In the example below both
         * parameters are send as `character varying` but the server will
         * perform the cast to `date` and `integer`.
         * ```
           cnx.execute("UPDATE titles SET to_date=$1::date WHERE emp_no::integer=$2", "1988-02-10", "10020");
         * ```
         *
         * The `null` value can be send as a parameter using the `nullptr` keyword.
         * ```
         * cnx.execute("INSERT INTO titles VALUES ($1, $2, $3::date, $4)",
         *             10020, "Technique Leader", "1988-02-10", nullptr);
         * ```
         *
         * @attention Parameters are only supported for a single SQL command. If
         *            execute() is called with more than one sql command and any
         *            of those commands are using parameters, execute() will fail.
         *
         * @return The results of the SQL commands.
         *
         * You can iterate over the returned result using the Result::iterator.
         * When a query in a the `sql` command is a SELECT all the rows of the
         * result must be fetched using the iterator before the connection can
         * be reused for another command. For other commands such as an UPDATE
         * or a CREATE TABLE, using the iterator is not required and the
         * connection can be reused for the next command right away.
         *
         **/
        template<typename... Args>
        Result &execute(const char *sql, Args... args) {
          Params params(settings_, sizeof...(args));
          std::make_tuple((params.bind(std::forward<Args>(args)), 0)...);
          execute(sql, params);
          return result_;
        }

        /**
         * Start a transaction.
         *
         * begin(), commit() and rollback() are helper methods to deal with
         * transactions.
         * The main benefit of using those methods rather than executing the SQL
         * commands is on nested transactions. If a code start a transaction and
         * call another code also starting a transaction, thoses methods will
         * create only one transaction started at the first all to begin() and
         * commited at the last call to commit().
         *
         * @return The connection itself.
         **/
        Connection &begin();

        /**
         * Commit a transaction.
         *
         * @return The connection itself.
         **/
        Connection &commit();

        /**
         * Rollback a transaction.
         *
         * @return The connection itself.
         **/
        Connection &rollback();

      protected:

        PGconn *pgconn_;  /**< The native connection pointer. **/
        Result  result_;  /**< Result of the current query. **/

        /**
         * Last error messages sent by the server.
         *
         * @return The error message most recently generated by an operation on
         *         the connection.
         **/
        std::string lastError() const noexcept;

        /**
         * Cast operator to the native connection pointer.
         **/
        operator PGconn *() {
          return pgconn_;
        }

      private:

        /**
         * Connection settings.
         **/
        Settings settings_;

        /**
         * Current transaction level.
         *
         * * 0 → no transaction in progress.
         * * 1 → one transaction in progress.
         * * 2 → one transaction in progress + 1 nested transaction.
         * * n → one transaction in progress + (n-1) nested transactions.
         *
         **/
        int transaction_;

        /**
         * Private implementation of the exectute public method.
         **/
        void execute(const char *sql, const Params &params);

        Connection(const Connection&) = delete;
        Connection(const Connection&&) = delete;
        Connection& operator = (const Connection&) = delete;
        Connection& operator = (const Connection&&) = delete;
    };
    
    /**
     * Check if a sql command contains one or more statements.
     *
     * This method does not perform a strict parsing of the SQL. If the `sql`
     * parameter is syntactically incorrect, the return value of this method
     * is undetermined.
     *
     * @param sql The SQL commands to parse.
     * @return true if more than one statement have been found.
     **/
    bool isSingleStatement(const char *sql) noexcept;

  } // namespace postgres  
}   // namespace db
