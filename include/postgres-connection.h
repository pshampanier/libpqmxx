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
         * postgresql://[user[:password]@][netloc][:port][/dbname][?param1=value1&...]
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
          Params params(sizeof...(args));
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

        Connection &once(std::function<bool (const Result &result)> callback);
        Connection &each(std::function<bool (const Result &result)> callback);
        Connection &done(std::function<void (int count)> callback);
        Connection &always(std::function<void ()> callback);
        Connection &error(std::function<void (std::exception_ptr reason)> callback);

      protected:

        PGconn *pgconn_;  /**< The native connection pointer. **/
        Result  result_;  /**< Result of the current query. **/
        bool    async_;   /**< `true` when running on asynchronous mode **/

        /**
         * Callbacks for asynchonous operations in non bloking mode.
         **/
        std::function<bool (Result &)>           iterator_; // once and each
        std::function<void (int)>                done_;
        std::function<void (std::exception_ptr)> error_;
        std::function<void ()>                   always_;

        /**
         * Process available data retreived from the server.
         *
         * This method must be called by the owner of the event loop when
         * some data are available in the connection.
         *
         * @return true if more data are expected. In this case the event loop
         *         owner must call again `consumeInput()` when more data will
         *         become available.
         **/
        bool consumeInput();

        /**
         * Flush the data pending to be send throught the connection.
         *
         * This method must be called by the event loop owner when the
         * connection is ready to accept to write data to the server.
         *
         * @return true if not all data have been flushed. In this case the
         *         event loop owner must call again `flush()` when the server
         *         is ready to access more data.
         **/
        bool flush();

        /**
         * Check the connection progress status.
         **/
        void connectPoll();

        virtual void asyncRead(int socket, std::function<void ()> handler) const noexcept {}
        virtual void asyncWrite(int socket, std::function<void ()> handler) const noexcept {}

        /**
         * Get the native socket identifier.
         *
         * See [PQsocket](https://www.postgresql.org/docs/current/static/libpq-status.html).
         * @return The file descriptor number of the connection socket to the server.
         **/
        int socket() const noexcept;

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

  } // namespace postgres  
}   // namespace db
