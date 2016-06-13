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
#include <cstdlib>

#ifndef NDEBUG
  #include "libpq/pg_type.h"
#endif

#ifdef __linux__
  #include <endian.h>
#elif defined (__APPLE__)
  #include <libkern/OSByteOrder.h>
  #define be16toh(x) OSSwapBigToHostInt16(x)
  #define be32toh(x) OSSwapBigToHostInt32(x)
  #define be64toh(x) OSSwapBigToHostInt64(x)
#elif defined (__WINDOWS__)
  #include <winsock2.h>
  #include <sys/param.h>
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

    int Row::num() const noexcept {
      return result_.num_;
    }

    template<>
    bool Row::get<bool>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert(PQftype(result_, column) == BOOLOID);
      return *reinterpret_cast<bool *>(PQgetvalue(result_, 0, column));
    }

    template<>
    int16_t Row::get<int16_t>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert(PQftype(result_, column) == INT2OID);
      return be16toh(*reinterpret_cast<int16_t*>(PQgetvalue(result_, 0, column)));
    }

    template<>
    int32_t Row::get<int32_t>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert(PQftype(result_, column) == INT4OID);
      return be32toh(*reinterpret_cast<int32_t*>(PQgetvalue(result_, 0, column)));
    }

    template<>
    int64_t Row::get<int64_t>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert(PQftype(result_, column) == INT8OID);
      return be64toh(*reinterpret_cast<int64_t*>(PQgetvalue(result_, 0, column)));
    }

    template<>
    float Row::get<float>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert(PQftype(result_, column) == FLOAT4OID);
      uint32_t v = be32toh(*reinterpret_cast<int32_t*>(PQgetvalue(result_, 0, column)));
      return *reinterpret_cast<float*>(&v);
    }

    // -------------------------------------------------------------------------
    // double precision
    // -------------------------------------------------------------------------
    template<>
    double Row::get<double>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert(PQftype(result_, column) == FLOAT8OID);
      uint64_t v = be64toh(*reinterpret_cast<int64_t*>(PQgetvalue(result_, 0, column)));
      return *reinterpret_cast<double*>(&v);
    }

    // -------------------------------------------------------------------------
    // char, varchar, text
    // -------------------------------------------------------------------------
    template<>
    std::string Row::get<std::string>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert(PQftype(result_, column) == BPCHAROID ||
             PQftype(result_, column) == VARCHAROID ||
             PQftype(result_, column) == NAMEOID ||
             PQftype(result_, column) == TEXTOID);
      return std::string(PQgetvalue(result_, 0, column));
    }

    // -------------------------------------------------------------------------
    // "char"
    // -------------------------------------------------------------------------
    template<>
    char Row::get<char>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert(PQftype(result_, column) == CHAROID);
      char c = *PQgetvalue(result_, 0, column);
      return c;
    }

    // -------------------------------------------------------------------------
    // bytea
    // -------------------------------------------------------------------------
    template<>
    std::vector<uint8_t> Row::get<std::vector<uint8_t>>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert(PQftype(result_, column) == BYTEAOID);
      int length = PQgetlength(result_, 0, column);
      uint8_t *data = reinterpret_cast<uint8_t *>(PQgetvalue(result_, 0, column));
      return std::vector<uint8_t>(data, data + length);
    }

    // -------------------------------------------------------------------------
    // Result contructor
    // -------------------------------------------------------------------------
    Result::Result(Connection &conn)
      : Row(*this), conn_(conn), begin_(*this), end_(*this) {
      pgresult_ = nullptr;
      status_ = PGRES_EMPTY_QUERY;
    }

    // -------------------------------------------------------------------------
    // Destructor
    // -------------------------------------------------------------------------
    Result::~Result() {
      if (pgresult_) {
        PQclear(pgresult_);
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
      if (ptr_->result_.pgresult_ == nullptr) {
        // We've reached the end
        ptr_ = &ptr_->result_.end_;
      }
      return iterator (ptr_);
    }

    // -------------------------------------------------------------------------
    // Number of rows affected by the SQL command
    // -------------------------------------------------------------------------
    uint64_t Result::count() const noexcept {
      assert(pgresult_);
      const char *count = PQcmdTuples(pgresult_);
      char *end;
      return std::strtoull(count, &end, 10);
    }

    // -------------------------------------------------------------------------
    // Get the first result from the server.
    // -------------------------------------------------------------------------
    void Result::first() {
      assert(pgresult_ == nullptr);
      num_ = 0;
      next();
    }

    // -------------------------------------------------------------------------
    // Get the next result from the server.
    // -------------------------------------------------------------------------
    void Result::next() {

      if (pgresult_) {
        assert(status_ == PGRES_SINGLE_TUPLE);
        PQclear(pgresult_);
      }

      pgresult_ = PQgetResult(conn_);
      assert(pgresult_);
      status_ = PQresultStatus(pgresult_);
      switch (status_) {
        case PGRES_SINGLE_TUPLE:
          assert(PQntuples(pgresult_) == 1);
          num_++;
          break;

        case PGRES_TUPLES_OK:
          // The SELECT statement did not return any row or after the last row,
          // a zero-row object with status PGRES_TUPLES_OK is returned; this is
          // the signal that no more rows are expected.
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

        case PGRES_COMMAND_OK:
          break;

        default:
          assert(false);
          break;
      }

    }

    // -------------------------------------------------------------------------
    // Clear the previous result of the connection
    // -------------------------------------------------------------------------
    void Result::clear() {

      switch (status_) {
        case PGRES_COMMAND_OK:
          PQclear(pgresult_);
          pgresult_ = PQgetResult(conn_);
          assert(pgresult_ == nullptr);
          break;

        case PGRES_SINGLE_TUPLE:
          next();
          if (status_ == PGRES_SINGLE_TUPLE) {
            // All results of the previous query have not been processed, we
            // need to cancel it.
            conn_.cancel();
          }
          assert(pgresult_ == nullptr);
          break;

        case PGRES_EMPTY_QUERY:
          break;

        default:
          assert(true);
      }

    }

  } // namespace postgres
}   // namespace db
