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

#include <cassert>

#ifndef NDEBUG
  #include "debug/pg_type.h"
#endif

#ifdef __linux__
  #include <endian.h>
#elif defined (__APPLE__)
  #include <libkern/OSByteOrder.h>
  #define be16toh(x) OSSwapBigToHostInt16(x)
  #define be32toh(x) OSSwapBigToHostInt32(x)
  #define be64toh(x) OSSwapBigToHostInt64(x)
#elif defined (__WINDOWS__)
  #define be16toh(x) ntohs(x)
  #define be32toh(x) ntohl(x)
  #define be64toh(x) ntohll(x)
#endif

namespace db {
  namespace postgres {

    // -------------------------------------------------------------------------
    // Row contructor
    // -------------------------------------------------------------------------
    Row::Row(Result &result)
    : result_(result) {
    }

    // -------------------------------------------------------------------------
    // Row contructor
    // -------------------------------------------------------------------------
    /*
    Row::Row(Row &row)
    : result_(row.result_) {
    }*/

    template<>
    bool Row::get<bool>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert(PQftype(result_, column) == BOOLOID);
      return *reinterpret_cast<bool *>(PQgetvalue(result_, 0, column));
    }

    template<>
    int32_t Row::get<int32_t>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert(PQftype(result_, column) == INT4OID);
      return be32toh(*reinterpret_cast<int32_t*>(PQgetvalue(result_, 0, column)));
    }

    // -------------------------------------------------------------------------
    // Result contructor
    // -------------------------------------------------------------------------
    Result::Result(Connection &conn)
      : Row(*this), conn_(conn), begin_(*this), end_(*this) {
      pgresult_ = nullptr;
      pgnext_ = nullptr;
      status_ = PGRES_EMPTY_QUERY;
    }

    // -------------------------------------------------------------------------
    // Destructor
    // -------------------------------------------------------------------------
    Result::~Result() {
      if (pgresult_) {
        PQclear(pgresult_);
      }
      if (pgnext_) {
        PQclear(pgnext_);
      }
    }

    // -------------------------------------------------------------------------
    // First row of the result
    // -------------------------------------------------------------------------
    Result::iterator Result::begin() {
      return Result::iterator(&begin_);
    }

    // -------------------------------------------------------------------------
    // Last row of the result
    // -------------------------------------------------------------------------
    Result::iterator Result::end() {
      // If there is no result available then end() = begin()
      return Result::iterator(pgresult_ ? &end_ : &begin_);
    }

    // -------------------------------------------------------------------------
    // Next row of the result
    // -------------------------------------------------------------------------
    Result::iterator Result::iterator::operator ++() {
      ptr_->result_.next();
      if (ptr_->result_.pgnext_ == nullptr) {
        // This is the last one
        ptr_ = &ptr_->result_.end_;
      }
      return iterator (ptr_);
    }

    void Result::operator = (PGresult *pgresult) {
      clear();
      pgresult_ = pgresult;
      status_ = PQresultStatus(pgresult_);
      switch (status_) {
        case PGRES_BAD_RESPONSE:
        case PGRES_FATAL_ERROR:
          throw ExecutionException(conn_.lastError());
          break;

        case PGRES_TUPLES_OK:
        case PGRES_SINGLE_TUPLE:
          if (PQntuples(pgresult_) == 0) {
            // A SELECT statement has produced zero result, we can free the result
            PQclear(pgresult_);
            pgresult_ = PQgetResult(conn_);
            assert(pgresult_ == nullptr);
          }
          else {
            // Get the next result to ckeck if the current result is the last
            // one of the resultset.
            pgnext_ = PQgetResult(conn_);
            int nextStatus = PQresultStatus(pgnext_);
            if (nextStatus == PGRES_TUPLES_OK) {


            }
          }
          break;

        default:
          break;
      }
    }

    // -------------------------------------------------------------------------
    // Get the first result from the server.
    // -------------------------------------------------------------------------
    void Result::first() {
      assert(pgresult_ == nullptr && pgnext_ == nullptr);
      pgresult_ = PQgetResult(conn_);
      assert(pgresult_);
      status_ = PQresultStatus(pgresult_);
      switch (status_) {
        case PGRES_SINGLE_TUPLE:
          assert(PQntuples(pgresult_) == 1);
          pgnext_ = PQgetResult(conn_);
          assert(pgnext_);
          switch (PQresultStatus(pgnext_)) {
            case PGRES_SINGLE_TUPLE:
              assert(PQntuples(pgnext_) == 1);
              break;

            case PGRES_TUPLES_OK:
              // After the last row, a zero-row object with status PGRES_TUPLES_OK is
              // returned; this is the signal that no more rows are expected.
              assert(PQntuples(pgnext_) == 0);
              pgnext_ = PQgetResult(conn_);
              assert(pgnext_ == nullptr);
              break;

            case PGRES_BAD_RESPONSE:
            case PGRES_FATAL_ERROR:
              throw ExecutionException(conn_.lastError());
              break;

            default:
              assert(true);
              break;
          }
          break;

        case PGRES_TUPLES_OK:
          // The SELECT statement did not return any row.
          assert(PQntuples(pgresult_) == 0);
          PQclear(pgresult_);
          pgresult_ = PQgetResult(conn_);
          assert(pgresult_ == nullptr);
          status_ = PGRES_EMPTY_QUERY;
          break;

        case PGRES_BAD_RESPONSE:
        case PGRES_FATAL_ERROR:
          throw ExecutionException(conn_.lastError());
          break;

        default:
          break;
      }


    }

    // -------------------------------------------------------------------------
    // Get the next result from the server.
    // -------------------------------------------------------------------------
    void Result::next() {

      assert(pgresult_);
      PQclear(pgresult_);
      pgresult_ = pgnext_;
      if (pgresult_) {
        pgnext_ = PQgetResult(conn_);
        auto nextStatus = PQresultStatus(pgnext_);
        switch (nextStatus) {
          case PGRES_TUPLES_OK:
            // After the last row, a zero-row object with status PGRES_TUPLES_OK is
            // returned; this is the signal that no more rows are expected.
            assert(PQntuples(pgnext_) == 0);
            pgnext_ = PQgetResult(conn_);
            assert(pgnext_ == nullptr);
            break;

          case PGRES_BAD_RESPONSE:
          case PGRES_FATAL_ERROR:
            throw ExecutionException(conn_.lastError());
            break;

          default:
            assert(nextStatus == PGRES_SINGLE_TUPLE);
        }
      }
      else {
        status_ = PGRES_EMPTY_QUERY;
      }

    }

    // -------------------------------------------------------------------------
    // Clear the previous result of the connection
    // -------------------------------------------------------------------------
    void Result::clear() {

      switch (status_) {
        case PGRES_SINGLE_TUPLE:
          PQclear(pgresult_);
          if (pgnext_ != nullptr) {
            // The previous query is still in progress, we need to cancel it.
            PQclear(pgnext_);
            pgnext_ = nullptr;
            // ## TO DO: conn_.cancel();
          }
          pgresult_ = nullptr;
          break;

        default:
          assert(true);
      }

    }

  } // namespace postgres
}   // namespace db
