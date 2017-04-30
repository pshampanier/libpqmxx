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
#include "postgres-statements.h"

#include <functional>
#include <memory>
#include <cstddef>

namespace libpqmxx {

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

    /**
     * Client encoding.
     **/
    std::string encoding = "utf8";
  };
  
  enum class action {
    
    connect,  /**< Connection to the server has establised. **/
    close,    /**< Connection to the server closed. **/
    reset,    /**< Connection reset to the server initiated. **/
    read,     /**< Connection read pending. **/
    write,     /**< Connection write pending. **/
    read_write
    
  };
  
  /**
   * Handler for asynchronous operations.
   **/
  typedef std::function<void (std::exception_ptr &reason) noexcept> handler_t;

  /**
   * Handler for postgresql notices.
   **/
  typedef std::function<void (const char *message) noexcept> notice_handler_t;
  
  /**
   * A connection to a PostgreSQL database.
   **/
  class Connection : public std::enable_shared_from_this<Connection> {

    friend class Result;
    friend class ResultHandle;

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
    virtual ~Connection();

    /**
     * Asynchronous mode.
     *
     * @return true if the connection is in non-blocking mode.
     **/
    bool async() const noexcept {
      return async_;
    }

    /**
     * Open a connection to the database.
     *
     * @param connInfo A postgresql connection string.
     *                 Both Keyword/Value and URI are accepted.
     *                 The passed `connInfo` can be empty or null to use all
     *                 default parameters
     *                 (see [Environment Variables](https://www.postgresql.org/docs/9.5/static/libpq-envars.html)).
     *
     * @param handler  Used by asynchronous connection only. This handler will
     *                 be called when the connnection has been successfully
     *                 completed or failed. The success of failure of the
     *                 connection can be checked by using the parameter `eptr`
     *                 provided to the handler. When the connection is a success
     *                 `eptr` is nullptr otherwise it describes the reason of
     *                 the connection failure.
     *
     *                 The handler should not throw any exception.
     *
     *                 The handler may never be called. In non blocking mode,
     *                 the `connect_timeout` parameter provided in `connInfo`
     *                 is ignored and this is the responsability of the
     *                 application to handle the timeout.
     *
     * ```
     * postgresql://[user[:password]@][netloc][:port][/dbname][?param1=value1&...]
     * ```
     *
     * @see https://www.postgresql.org/docs/9.5/static/libpq-connect.html#LIBPQ-CONNSTRING
     * @return The connection itself.
     **/
    Connection &connect(const char *connInfo = nullptr, handler_t handler = nullptr);

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
       date                        | libpqmxx::date_t
       time without time zone      | libpqmxx::time_t
       timestamp without time zone | libpqmxx::timestamp_t
       timestamp with time zone    | libpqmxx::timestamptz_t
       interval                    | libpqmxx::interval_t
       time with time zone         | libpqmxx::timetz_t

     *
     * \code

        // Execute a query
        auto results = cnx.execute("SELECT MAX(emp_no) FROM employees");

        // Execute a query with parameters.
        cnx.execute("UPDATE titles SET to_date=$1::date WHERE emp_no=$2", "1988-02-10", 10020);

     * \code
     *
     * When the parameter data type might be ambigious for the server, you
     * can use the PostgreSQL casting syntax. In the example below both
     * parameters are send as `character varying` but the server will
     * perform the cast to `date` and `integer`.
     *
     * \code
     *
        cnx.execute("UPDATE titles SET to_date=$1::date WHERE emp_no::integer=$2", "1988-02-10", "10020");
     *
     * \code
     *
     * The `null` value can be send as a parameter using the `nullptr` keyword.
     *
     * \code
     *
          cnx.execute("INSERT INTO titles VALUES ($1, $2, $3::date, $4)",
                      10020, "Technique Leader", "1988-02-10", nullptr);
     *
     * \code
     *
     * @return The results of the SQL command.
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
    Result execute(const char *sql, Args... args) {
      Params params(settings_, sizeof...(args));
      std::make_tuple((params.bind(std::forward<Args>(args)), 0)...);
      return execute(sql, params);
    }

    /**
     * Execute a batch of SQL commands.
     *
     * \code

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
       
       )SQL"_x);

     * \code
     *
     * To produce a BatchStatement from a string literal, use the user defined
     * string literal `_x`.
     *
     * The batch of sql operations is applied atomically.
     *
     * @return The result of the execution.
     *
     *  - In synchonous mode, only the result of the last command is returned.
     *  - In asynchonous mode, done() is called for each sql statement in the
     *    batch. If any of the statements returns rows, they can be reached
     *    using the each() method on the returned Result.
     **/
    Result execute(const BatchStatement &sql);

    /**
     * Execute the next statement in single row mode.
     *
     * By default when calling execute(), the entire result is fetched in
     * memory and returned by the caller. While this is the most efficient
     * way for most of the queries, this can be unsutable for large results.
     * When the single row mode is enable, the Result returned by exectute()
     * contains only the first row, next rows will be fetched one at a time
     * while iterating on the result.
     *
     * This method must be called right before execute() and will apply only
     * on the next execute().
     *
     * In single row mode, the all rows must have been fetched before calling
     * execute() for a next query.
     *
     * @return The connection itself.
     **/
    Connection &setSingleRowMode() noexcept;
    
    /**
     * Notice processing.
     *
     * Notice and warning messages generated by the server that do not imply
     * the failure of the query can be captured by registering an handler.
     *
     * By default, notices are displayed on std::err.
     *
     * @param handler A handler to be called on server notices.
     *                Passing nullptr will totaly disable the notices.
     *
     * @return The connection itself.
     **/
    Connection &notice(notice_handler_t &&handler) noexcept;

  protected:

    PGconn                      *pgconn_;       /**< The native connection pointer. **/
    std::weak_ptr<ResultHandle> lastResult_;    /**< Result of the last execution. **/
    bool                        async_;         /**< `true` when running on asynchronous mode. **/
    bool                        singleRowMode_; /**< `true` when SetSingleRowMode() has been called. **/

    /**
     * Handler for asynchonous operations in non blocking mode.
     **/
    handler_t handler_;

    /**
     * Process available data retreived from the server.
     *
     * This method must be called by the owner of the event loop when
     * some data are available in the connection.
     **/
    void consumeInput();

    /**
     * Flush the data pending to be send throught the connection.
     *
     * This method must be called by the event loop owner when the
     * connection is ready to accept to write data to the server.
     **/
    void flush();

    /**
     * Check the connection progress status.
     **/
    void connectPoll();

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
    std::string lastPostgresError() const noexcept;

    /**
     * Cast operator to the native connection pointer.
     **/
    operator PGconn *() {
      return pgconn_;
    }

    /**
     * Trigger an action for the layer in charge of asynchonous I/O.
     *
     * The implementation must not throw any exception but rather use the
     * callback to report an error.
     **/
    virtual void asyncAction(action action, handler_t callback) noexcept;

  private:

    Settings settings_;               /**< Connection settings. **/
    notice_handler_t notice_handler_; /**< Notice Processing handler. **/

    /**
     * Private implementation of the execute public method.
     **/
    Result execute(const char *sql, const Params &params);

    /**
     * Asynchronous connection completed.
     *
     * @eptr nullptr if the connection was successful, otherwise the reason
     *       of the connection failure.
     **/
    void connectCompleted(std::exception_ptr eptr = nullptr) noexcept;

    Connection(const Connection&) = delete;
    Connection(const Connection&&) = delete;
    Connection& operator = (const Connection&) = delete;
    Connection& operator = (const Connection&&) = delete;
  };

} // namespace libpqmxx
