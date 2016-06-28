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
#include "gtest/gtest.h"
#include "postgres-connection.h"
#include "postgres-exceptions.h"

using namespace db::postgres;

TEST(result_sync, integer_types) {

  Connection cnx;
  cnx.connect("postgresql://ci-test@localhost");

  EXPECT_EQ(cnx.execute("SELECT true").get<bool>(0), true);
  EXPECT_EQ(cnx.execute("SELECT false").get<bool>(0), false);
  EXPECT_EQ(cnx.execute("SELECT CAST(32767 AS SMALLINT)").get<int16_t>(0), 32767);
  EXPECT_EQ(cnx.execute("SELECT 2147483647").get<int32_t>(0), 2147483647);
  EXPECT_EQ(cnx.execute("SELECT 9223372036854775807").get<int64_t>(0), 9223372036854775807);
  EXPECT_FLOAT_EQ(cnx.execute("SELECT CAST(4.46678 AS REAL)").get<float>(0), 4.46678);
  EXPECT_FLOAT_EQ(cnx.execute("SELECT CAST(4.46678 AS DOUBLE PRECISION)").get<double>(0), 4.46678);

}

TEST(result_sync, floating_point_types) {

  Connection cnx;
  cnx.connect("postgresql://ci-test@localhost");

  EXPECT_FLOAT_EQ(cnx.execute("SELECT CAST(4.46678 AS REAL)").get<float>(0), 4.46678);
  EXPECT_FLOAT_EQ(cnx.execute("SELECT CAST(4.46678 AS DOUBLE PRECISION)").get<double>(0), 4.46678);

}

TEST(result_sync, serial_types) {

  Connection cnx;
  cnx.connect("postgresql://ci-test@localhost");

  cnx.execute("CREATE TABLE tmp(id16 smallserial, id32 serial, id64 bigserial, val integer)");
  cnx.execute("INSERT INTO tmp(val) SELECT 0");
  Result &result = cnx.execute("SELECT id16, id32, id64 FROM tmp");
  EXPECT_EQ(result.get<int16_t>(0), 1);
  EXPECT_EQ(result.get<int32_t>(1), 1);
  EXPECT_EQ(result.get<int64_t>(2), 1);
  cnx.execute("DROP TABLE tmp");

}

TEST(result_sync, char_types) {

  Connection cnx;
  cnx.connect("postgresql://ci-test@localhost");

  EXPECT_STREQ("hello     ", cnx.execute("SELECT CAST('hello' AS CHAR(10))").get<std::string>(0).c_str());
  EXPECT_STREQ("world", cnx.execute("SELECT CAST('world' AS VARCHAR(10))").get<std::string>(0).c_str());
  EXPECT_STREQ("hello world", cnx.execute("SELECT CAST('hello world' AS TEXT)").get<std::string>(0).c_str());
  EXPECT_EQ('X', cnx.execute("SELECT CAST('X' AS \"char\")").get<char>(0));
  EXPECT_STREQ("name", cnx.execute("SELECT CAST('name' AS NAME)").get<std::string>(0).c_str());

}

TEST(result_sync, utf8) {

  Connection cnx;
  cnx.connect("postgresql://ci-test@localhost");

  EXPECT_STREQ(u8"Günter", cnx.execute("SELECT 'Günter'").get<std::string>(0).c_str());
  EXPECT_STREQ(u8"メインページ", cnx.execute("SELECT 'メインページ'").get<std::string>(0).c_str());

}

TEST(result_sync, date_time_types) {

  Connection cnx;
  cnx.connect("postgresql://ci-test@localhost");

  EXPECT_EQ(0, cnx.execute("SELECT TIMESTAMP WITH TIME ZONE '1970-01-01 00:00:00+00'").get<timestamptz_t>(0));
  EXPECT_EQ(600123, cnx.execute("SELECT TIMESTAMP WITH TIME ZONE '1970-01-01 00:00:00.600123+00'").get<timestamptz_t>(0));
  EXPECT_EQ(0, cnx.execute("SELECT DATE '1970-01-01'").get<date_t>(0));
  EXPECT_EQ(1451606400, cnx.execute("SELECT DATE '2016-01-01'").get<date_t>(0));

  auto timetz = cnx.execute("SELECT TIME WITH TIME ZONE '00:00:01.000001-07'").get<timetz_t>(0);
  EXPECT_EQ(1000001, timetz.time);
  EXPECT_EQ(7*3600, timetz.offset);

  EXPECT_EQ(39602000101, cnx.execute("SELECT TIME '11:00:02.000101'").get<db::postgres::time_t>(0));

  auto interval = cnx.execute("SELECT INTERVAL '3 months 7 days 2:03:04'").get<interval_t>(0);
  EXPECT_EQ(7384000000, interval.time);
  EXPECT_EQ(7, interval.days);
  EXPECT_EQ(3, interval.months);

  cnx.execute("set timezone TO 'GMT'");
  EXPECT_EQ(cnx.execute("SELECT TIMESTAMP '1970-01-01 00:00:00.600123'").get<timestamp_t>(0), 600123);

}

TEST(result_sync, bytea_type) {

  Connection cnx;
  cnx.connect("postgresql://ci-test@localhost");

  std::vector<uint8_t> expected;
  expected.push_back(0xDE);
  expected.push_back(0xAD);
  expected.push_back(0xBE);
  expected.push_back(0xEF);

  std::vector<uint8_t> actual = cnx.execute("SELECT CAST(E'\\\\xDEADBEEF' AS BYTEA)").get<std::vector<uint8_t>>(0);
  EXPECT_TRUE(expected.size() == actual.size() && std::equal(actual.begin(), actual.end(), expected.begin()));

}

TEST(result_sync, null_values) {

  Connection cnx;
  cnx.connect("postgresql://ci-test@localhost");

  EXPECT_TRUE(cnx.execute("SELECT NULL::bigint").isNull(0));
  EXPECT_FALSE(cnx.execute("SELECT NULL::bool").get<bool>(0));
  EXPECT_EQ(0, cnx.execute("SELECT NULL::smallint").get<int16_t>(0));
  EXPECT_EQ(0, cnx.execute("SELECT NULL::integer").get<int32_t>(0));
  EXPECT_EQ(0., cnx.execute("SELECT NULL::float4").get<float>(0));
  EXPECT_EQ(0., cnx.execute("SELECT NULL::float8").get<double>(0));
  EXPECT_EQ(0, cnx.execute("SELECT NULL::bytea").get<std::vector<uint8_t>>(0).size());
  EXPECT_EQ(0, cnx.execute("SELECT NULL::date").get<date_t>(0));
  EXPECT_EQ(0, cnx.execute("SELECT NULL::timestamptz").get<timestamptz_t>(0));
  EXPECT_EQ(0, cnx.execute("SELECT NULL::timestamp").get<timestamp_t>(0));
  EXPECT_EQ(0, cnx.execute("SELECT NULL::time").get<db::postgres::time_t>(0));
  auto timetz = cnx.execute("SELECT NULL::timetz").get<db::postgres::timetz_t>(0);
  EXPECT_EQ(0, timetz.time);
  EXPECT_EQ(0, timetz.offset);
  auto interval = cnx.execute("SELECT NULL::interval").get<db::postgres::interval_t>(0);
  EXPECT_EQ(0, interval.time);
  EXPECT_EQ(0, interval.days);
  EXPECT_EQ(0, interval.months);

}

TEST(result_sync, column_name) {

  Connection cnx;
  cnx.connect("postgresql://ci-test@localhost");

  auto &result = cnx.execute("SELECT 0 AS c1, 1, 2 AS \"Günter\", 3 AS \"メインページ\"");
  EXPECT_STREQ("c1", result.columnName(0));
  EXPECT_STREQ("?column?", result.columnName(1));
  EXPECT_STREQ(u8"Günter", result.columnName(2));
  EXPECT_STREQ(u8"メインページ", result.columnName(3));

}
