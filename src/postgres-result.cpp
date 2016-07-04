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
#include <cstring>
#include <iostream>

#include "libpq/pg_type.h"

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

#ifdef NDEBUG

    #define assert_oid(expected, actual) ((void)0)

#else

    void assert_oid(int expected, int actual) {
      const char *_expected = nullptr;
      if (expected != actual) {
        switch (expected) {
          case BOOLOID: _expected = "std::vector<uint_8>"; break;
          case BYTEAOID: _expected = "char"; break;
          case CHAROID: _expected = "std::string"; break;
          case NAMEOID: _expected = "std::string"; break;
          case INT8OID: _expected = "int64_t"; break;
          case INT2OID: _expected = "int16_t"; break;
          case INT4OID: _expected = "int32_t"; break;
          case TEXTOID: _expected = "std::string"; break;
          case FLOAT4OID: _expected = "float"; break;
          case FLOAT8OID: _expected = "double"; break;
          case BPCHAROID: _expected = "std::string"; break;
          case VARCHAROID: _expected = "std::string"; break;
          case DATEOID: _expected = "db::postgres::date_t"; break;
          case TIMEOID: _expected = "db::postgres::time_t"; break;
          case TIMESTAMPOID: _expected = "db::postgres::timestamp_t"; break;
          case TIMESTAMPTZOID: _expected = "db::postgres::timestamptz_t"; break;
          case INTERVALOID: _expected = "db::postgres::interval_t"; break;
          case TIMETZOID: _expected = "db::postgres::timetz_t"; break;
          default:
            assert(false); // unsupported type. try std::string
        }

        std::cerr << "Unexpected C++ type. Please use get<" << _expected
          << ">(int column)" << std::endl;
        assert(false);
      }
    }
#endif

    int16_t read16(const PGresult *pgresult, int column) {
      return be16toh(*reinterpret_cast<int16_t*>(PQgetvalue(pgresult, 0, column)));
    }

    int32_t read32(const PGresult *pgresult, int column) {
      return be32toh(*reinterpret_cast<int32_t*>(PQgetvalue(pgresult, 0, column)));
    }

    int64_t read64(const PGresult *pgresult, int column) {
      return be64toh(*reinterpret_cast<int64_t*>(PQgetvalue(pgresult, 0, column)));
    }

    // -------------------------------------------------------------------------
    // Row contructor
    // -------------------------------------------------------------------------
    Row::Row(Result &result)
    : result_(result) {
    }

    int Row::num() const noexcept {
      return result_.num_;
    }

    // -------------------------------------------------------------------------
    // Tests a column for a null value.
    // -------------------------------------------------------------------------
    bool Row::isNull(int column) const {
      assert(result_.pgresult_ != nullptr);
      return PQgetisnull(result_, 0, column) == 1;
    }

    // -------------------------------------------------------------------------
    // Get a column name.
    // -------------------------------------------------------------------------
    const char *Row::columnName(int column) const {
      assert(result_.pgresult_ != nullptr);
      const char *res = PQfname(result_, column);
      assert(res);
      return res;
    }

    // -------------------------------------------------------------------------
    // bool
    // -------------------------------------------------------------------------
    template<>
    bool Row::get<bool>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert_oid(PQftype(result_, column), BOOLOID);
      return PQgetisnull(result_, 0, column) ? false :
        *reinterpret_cast<bool *>(PQgetvalue(result_, 0, column));
    }

    // -------------------------------------------------------------------------
    // smallint
    // -------------------------------------------------------------------------
    template<>
    int16_t Row::get<int16_t>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert_oid(PQftype(result_, column), INT2OID);
      return PQgetisnull(result_, 0, column) ? 0 : read16(result_, column);
    }

    // -------------------------------------------------------------------------
    // integer
    // -------------------------------------------------------------------------
    template<>
    int32_t Row::get<int32_t>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert_oid(PQftype(result_, column), INT4OID);
      return PQgetisnull(result_, 0, column) ? 0 : read32(result_, column);
    }

    // -------------------------------------------------------------------------
    // bigint
    // -------------------------------------------------------------------------
    template<>
    int64_t Row::get<int64_t>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert_oid(PQftype(result_, column), INT8OID);
      return PQgetisnull(result_, 0, column) ? 0 : read64(result_, column);
    }

    // -------------------------------------------------------------------------
    // real
    // -------------------------------------------------------------------------
    template<>
    float Row::get<float>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert_oid(PQftype(result_, column), FLOAT4OID);
      if (PQgetisnull(result_, 0, column)) {
        return 0.f;
      }
      int32_t v = read32(result_, column);
      return *reinterpret_cast<float*>(&v);
    }

    // -------------------------------------------------------------------------
    // double precision
    // -------------------------------------------------------------------------
    template<>
    double Row::get<double>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert_oid(PQftype(result_, column), FLOAT8OID);
      if (PQgetisnull(result_, 0, column)) {
        return 0.;
      }
      uint64_t v = read64(result_, column);
      return *reinterpret_cast<double*>(&v);
    }

    // -------------------------------------------------------------------------
    // char, varchar, text
    // -------------------------------------------------------------------------
    template<>
    std::string Row::get<std::string>(int column) const {
      assert(result_.pgresult_ != nullptr);
      if (PQgetisnull(result_, 0, column)) {
        return std::string();
      }
      int length = PQgetlength(result_, 0, column);
      return std::string(PQgetvalue(result_, 0, column), length);
    }

    // -------------------------------------------------------------------------
    // "char"
    // -------------------------------------------------------------------------
    template<>
    char Row::get<char>(int column) const {
      assert(result_.pgresult_ != nullptr);
      if (PQgetisnull(result_, 0, column)) {
        return '\0';
      }
      assert(PQgetlength(result_, 0, column) == 1);
      return *PQgetvalue(result_, 0, column);
    }

    // -------------------------------------------------------------------------
    // bytea
    // -------------------------------------------------------------------------
    template<>
    std::vector<uint8_t> Row::get<std::vector<uint8_t>>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert_oid(PQftype(result_, column), BYTEAOID);
      int length = PQgetlength(result_, 0, column);
      uint8_t *data = reinterpret_cast<uint8_t *>(PQgetvalue(result_, 0, column));
      return std::vector<uint8_t>(data, data + length);
    }

    // -------------------------------------------------------------------------
    // date
    // -------------------------------------------------------------------------
    template<>
    date_t Row::get<date_t>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert_oid(PQftype(result_, column), DATEOID);
      int32_t i = 0;
      if (!PQgetisnull(result_, 0, column)) {
        i = read32(result_, column);
        i = (i+DAYS_UNIX_TO_J2000_EPOCH) * 86400;
      }
      return date_t { i };
    }

    // -------------------------------------------------------------------------
    // timestamptz
    // -------------------------------------------------------------------------
    template<>
    timestamptz_t Row::get<timestamptz_t>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert_oid(PQftype(result_, column), TIMESTAMPTZOID);
      int64_t t = 0;
      if (!PQgetisnull(result_, 0, column)) {
        t = read64(result_, column) + MICROSEC_UNIX_TO_J2000_EPOCH;
      }
      return timestamptz_t { t };
    }

    // -------------------------------------------------------------------------
    // timestamp
    // -------------------------------------------------------------------------
    template<>
    timestamp_t Row::get<timestamp_t>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert_oid(PQftype(result_, column), TIMESTAMPOID);
      int64_t t = 0;
      if (!PQgetisnull(result_, 0, column)) {
        t = read64(result_, column) + MICROSEC_UNIX_TO_J2000_EPOCH;
      }
      return timestamp_t { t };
    }

    // -------------------------------------------------------------------------
    // timetz
    // -------------------------------------------------------------------------
    template<>
    timetz_t Row::get<timetz_t>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert_oid(PQftype(result_, column), TIMETZOID);
      timetz_t t;
      if (PQgetisnull(result_, 0, column)) {
        std::memset(&t, 0, 12);
      }
      else {
        std::memcpy(&t, PQgetvalue(result_, 0, column), 12);
        t.time = be64toh(t.time);
        t.offset = be32toh(t.offset);
      }
      return t;
    }

    // -------------------------------------------------------------------------
    // time
    // -------------------------------------------------------------------------
    template<>
    time_t Row::get<time_t>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert_oid(PQftype(result_, column), TIMEOID);
      int64_t t = 0;
      if (!PQgetisnull(result_, 0, column)) {
        t = read64(result_, column);
      }
      return time_t { t };
    }

    // -------------------------------------------------------------------------
    // interval
    // -------------------------------------------------------------------------
    template<>
    interval_t Row::get<interval_t>(int column) const {
      assert(result_.pgresult_ != nullptr);
      assert_oid(PQftype(result_, column), INTERVALOID);
      interval_t i;
      if (PQgetisnull(result_, 0, column)) {
        std::memset(&i, 0, sizeof(i));
      }
      else {
        std::memcpy(&i, PQgetvalue(result_, 0, column), sizeof(i));
        i.time = be64toh(i.time);
        i.days = be32toh(i.days);
        i.months = be32toh(i.months);
      }
      return i;
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
      return Result::iterator(status_ == PGRES_SINGLE_TUPLE ? &end_ : &begin_);
    }

    // -------------------------------------------------------------------------
    // Next row of the result
    // -------------------------------------------------------------------------
    Result::iterator Result::iterator::operator ++() {
      ptr_->result_.next();
      if (ptr_->result_.status_ != PGRES_SINGLE_TUPLE) {
        // We've reached the end
        assert(ptr_->result_.status_ == PGRES_TUPLES_OK);
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
          do {
            PQclear(pgresult_);
            pgresult_ = PQgetResult(conn_);
            if (pgresult_ == nullptr) {
              status_ = PGRES_EMPTY_QUERY;
            }
            else {
              status_ = PQresultStatus(pgresult_);
              switch (status_) {
                case PGRES_COMMAND_OK:
                  break;

                case PGRES_BAD_RESPONSE:
                case PGRES_FATAL_ERROR:
                  throw ExecutionException(conn_.lastError());
                  break;

                case PGRES_SINGLE_TUPLE:
                case PGRES_TUPLES_OK:
                  // It is not supported to execute a multi statement sql
                  // without fetching the result with the result iterator.
                  assert(true);
                  break;

                default:
                  assert(true);
                  break;
              }
            }
          } while (status_ != PGRES_EMPTY_QUERY);
          break;

        case PGRES_BAD_RESPONSE:
        case PGRES_FATAL_ERROR:
        case PGRES_TUPLES_OK:
          PQclear(pgresult_);
          pgresult_ = PQgetResult(conn_);
          assert(pgresult_ == nullptr);
          status_ = PGRES_EMPTY_QUERY;
          break;

        case PGRES_SINGLE_TUPLE:
          next();
          if (status_ == PGRES_SINGLE_TUPLE) {
            // All results of the previous query have not been processed, we
            // need to cancel it.
            conn_.cancel();
          }
          else if (status_ == PGRES_TUPLES_OK) {
            PQclear(pgresult_);
            pgresult_ = PQgetResult(conn_);
            status_ = PGRES_EMPTY_QUERY;
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
