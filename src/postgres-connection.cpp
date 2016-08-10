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
#include "postgres-connection.h"
#include "postgres-exceptions.h"

#include <functional>
#include <cassert>
#include <string>
#include <cstring>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Secur32.lib")

namespace db {
  namespace postgres {

    enum sql_scanner_state {
      start,
      statement,
      dollar_quoted_string_start_tag,   /* inside a start tag                 */
      dollar_quoted_string,             /* $tag$Dianne's horse$tag$           */
      dollar_quoted_string_end_tag,     /* inside a end tag                   */
      quoted_identifier,                /* UPDATE "my_table" SET "a" = 5;     */
      string,
      comment,                          /* -- sql comment */
      block_comment                     /* C-style block comments */
    };

    enum sql_scanner_token {
      none,
      digit,
      dollar,
      apostrophe,     /* single quote */
      quotation_mark, /* douple quotes */
      dash,
      semicolon,
      asterisk,
      slash,
      space,
      character,
      end_of_line,
      end_of_string
    };

    inline sql_scanner_token scan(sql_scanner_token previous, const char *s, int32_t *length) {
      sql_scanner_token token;
      char c;
      *length = 0;
      while (true) {
        (*length)++;
        c = *s++;
        switch (c) {
          case '$': return dollar;
          case '\'': return apostrophe;
          case '"': return quotation_mark;
          case '-': return dash;
          case ';': return semicolon;
          case '*': return asterisk;
          case '/': return slash;
          case '\0': return end_of_string;
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            token = digit;
            break;
          case ' ':
          case '\t':
            token = space;
            break;
          case '\r':
          case '\n':
            token = end_of_line;
            break;
          default:
            token = character;
            break;
        }
        if (token != previous) {
          break;
        }
      }
      return token;
    }

    // -------------------------------------------------------------------------
    // Check if the sql contains one or more statement.
    // -------------------------------------------------------------------------
    bool isSingleStatement(const char *sql) noexcept {
      int32_t length = 0;
      const char *tag = nullptr;      // start of the tag
      std::vector<std::string> tags;  // tags used by dollar quoted strings
      std::vector<sql_scanner_state> states;
      states.push_back(start);
      sql_scanner_state state;
      sql_scanner_token token, previous = none;
      do {
        token = scan(previous, sql, &length);
        state = states.back();
        switch (state) {
          case start:
          case statement:
            switch (token) {
              case dash: previous == dash ? states.push_back(comment) : ((void)0); break;
              case asterisk: previous == slash ? states.push_back(block_comment) : ((void)0); break;
              case apostrophe: states.push_back(string); break;
              case quotation_mark: states.push_back(quoted_identifier); break;
              case semicolon: state == statement ? states.push_back(start) : ((void)0); break;
              case dollar:
                states.push_back(dollar_quoted_string_start_tag);
                tag = sql + length;
                break;
              case slash:
              case space:
              case end_of_line:
              case end_of_string:
                break;
              default:
                if (state == start) {
                  if (states.size() > 1) {
                    return false;
                  }
                  states.pop_back();
                  states.push_back(statement);
                }
                break;
            }
            break;

          case dollar_quoted_string_start_tag:
            switch (token) {
              case dollar:
                tags.push_back(std::string(tag, sql-tag));
                states.pop_back();
                states.push_back(dollar_quoted_string);
                break;
              default: break;
            }
            break;

          case dollar_quoted_string:
            switch (token) {
              case dollar:
                states.pop_back();
                states.push_back(dollar_quoted_string_end_tag);
                tag = sql + length;
                break;
              default: break;
            }
            break;

          case dollar_quoted_string_end_tag:
            switch (token) {
              case dollar:
                if (sql - tag == tags.back().length()
                    && std::strncmp(tags.back().c_str(), tag, sql - tag) == 0) {
                  // end of tag found
                  tags.pop_back();
                  states.pop_back();
                  if (tags.size() > 0) {
                    // nested quoted strings.
                    states.push_back(dollar_quoted_string);
                  }
                }
                else {
                  // nested quoted strings.
                  // this was not a end tag but the start of a new tag
                  states.pop_back();
                  states.push_back(dollar_quoted_string);
                  states.push_back(dollar_quoted_string);
                  tags.push_back(std::string(tag, sql-tag));
                }
                break;
              case digit:
                if (previous == dollar) {
                  // parameter inside a quoted string (ex: $1).
                  states.pop_back();
                }
                break;
              default: break;
            }
            break;

          case quoted_identifier:
            switch (token) {
              case quotation_mark: previous != quotation_mark ? states.pop_back() : (void)(0);
              default: break;
            }
            break;

          case string:
            switch (token) {
              case apostrophe: previous != apostrophe ? states.pop_back() : (void)(0);
              default: break;
            }
            break;

          case comment:
            switch (token) {
              case end_of_line: states.pop_back();
              default: break;
            }
            break;

          case block_comment:
            switch (token) {
              case asterisk: previous == slash ? states.push_back(block_comment) : ((void)0); break; // nested comment
              case slash: previous == asterisk ? states.pop_back() : (void)(0);
              default: break;
            }
            break;

          default:
            break;
        }

        previous = token;
        sql += length;

      } while (token != end_of_string);

      return true;
    }

    // -------------------------------------------------------------------------
    // Constructor.
    // -------------------------------------------------------------------------
    Connection::Connection(Settings settings)
      : result_(*this) {
      pgconn_ = nullptr;
      transaction_ = 0;
      settings_ = settings;
    }

    // -------------------------------------------------------------------------
    // Destructor.
    // -------------------------------------------------------------------------
    Connection::~Connection() {
      PQfinish(pgconn_);
    }
    
    // -------------------------------------------------------------------------
    // Open a connection to the database.
    // -------------------------------------------------------------------------
    Connection &Connection::connect(const char *connInfo) {
      pgconn_ = PQconnectdb(connInfo == nullptr ? "" : connInfo);
      
      if( PQstatus(pgconn_) != CONNECTION_OK ) {
        PQfinish(pgconn_);
        pgconn_ = nullptr;
        throw ConnectionException(std::string(PQerrorMessage(pgconn_)));
      }

      return *this;
    }
    
    // -------------------------------------------------------------------------
    // Close the database connection.
    // -------------------------------------------------------------------------
    Connection &Connection::close() noexcept {
      assert(pgconn_);
      PQfinish(pgconn_);
      pgconn_ = nullptr;
      return *this;
    }

    // -------------------------------------------------------------------------
    // Last error message on the connection.
    // -------------------------------------------------------------------------
    std::string Connection::lastError() const noexcept {
      return std::string(PQerrorMessage(pgconn_));
    }

    // -------------------------------------------------------------------------
    // Execute an SQL statement.
    // -------------------------------------------------------------------------
    void Connection::execute(const char *sql, const Params &params) {

      result_.clear();

      int success;
      if (isSingleStatement(sql)) {
        success = PQsendQueryParams(pgconn_, sql, int(params.values_.size()),
                                      params.types_.data(),
                                      params.values_.data(),
                                      params.lengths_.data(),
                                      params.formats_.data(),
                                      1 /* binary results */);
      }
      else {
        assert(!params.values_.size()); // parameters are allowed only for single commands
        success = PQsendQuery(pgconn_, sql);
      }

      if (success) {
        // Switch to the single row mode to avoid loading the all result in memory.
        success = PQsetSingleRowMode(pgconn_);
        assert(success);
        result_.first();
      }

      if (!success) {
        throw ExecutionException(lastError());
      }
    }

    // -------------------------------------------------------------------------
    // Start a transaction.
    // -------------------------------------------------------------------------
    Connection &Connection::begin() {
      execute("BEGIN;");
      transaction_++;
      return *this;
    }

    // -------------------------------------------------------------------------
    // Commit a transaction.
    // -------------------------------------------------------------------------
    Connection &Connection::commit() {
      assert(transaction_ > 0);
      if (--transaction_ == 0) {
        execute("COMMIT;");
      }
      return *this;
    }

    // -------------------------------------------------------------------------
    // Rollback a transaction.
    // -------------------------------------------------------------------------
    Connection &Connection::rollback() {
      assert(transaction_ > 0);
      execute("ROLLBACK;");
      transaction_ = 0;
      return *this;
    }
    
    // -------------------------------------------------------------------------
    // Cancel queries in progress.
    // -------------------------------------------------------------------------
    Connection &Connection::cancel() {
      char errbuf[256];
      PGcancel *pgcancel = PQgetCancel(pgconn_);

      int success = PQcancel(pgcancel, errbuf, sizeof(errbuf));
      if (!success) {
        throw ExecutionException(errbuf);
      }

      PQfreeCancel(pgcancel);
      return *this;
    }

  } // namespace postgres
}   // namespace db
