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

#define ARRAY(...) __VA_ARGS__
#define TEST_VECTOR(expr, _expected, expected_size, type)                       \
{                                                                               \
  auto res = expr;                                                              \
  type exp[] = _expected;                                                       \
  EXPECT_EQ(res.size(), expected_size);                                         \
  for (size_t i=0; i < res.size() && expected_size == res.size(); i++) {        \
    auto actual = res[i];                                                       \
    auto expected = exp[i];

#define TEST_VECTOR_END                                                        \
  }                                                                            \
}

using namespace db::postgres;

TEST(result_sync, integer_types) {

  Connection cnx;
  cnx.connect();

  EXPECT_EQ(cnx.execute("SELECT true").as<bool>(0), true);
  EXPECT_EQ(cnx.execute("SELECT false").as<bool>(0), false);
  EXPECT_EQ(cnx.execute("SELECT CAST(32767 AS SMALLINT)").as<int16_t>(0), 32767);
  EXPECT_EQ(cnx.execute("SELECT 2147483647").as<int32_t>(0), 2147483647);
  EXPECT_EQ(cnx.execute("SELECT 9223372036854775807").as<int64_t>(0), 9223372036854775807);
  EXPECT_FLOAT_EQ(cnx.execute("SELECT CAST(4.46678 AS REAL)").as<float>(0), 4.46678f);
  EXPECT_DOUBLE_EQ(cnx.execute("SELECT CAST(4.46678 AS DOUBLE PRECISION)").as<double>(0), 4.46678);

}

TEST(result_sync, floating_point_types) {

  Connection cnx;
  cnx.connect();

  EXPECT_FLOAT_EQ(cnx.execute("SELECT CAST(4.46678 AS REAL)").as<float>(0), 4.46678f);
  EXPECT_DOUBLE_EQ(cnx.execute("SELECT CAST(4.46678 AS DOUBLE PRECISION)").as<double>(0), 4.46678);

}

TEST(result_sync, serial_types) {

  Connection cnx;
  cnx.connect();

  cnx.execute("CREATE TABLE tmp(id16 smallserial, id32 serial, id64 bigserial, val integer)");
  cnx.execute("INSERT INTO tmp(val) SELECT 0");
  Result &result = cnx.execute("SELECT id16, id32, id64 FROM tmp");
  EXPECT_EQ(result.as<int16_t>(0), 1);
  EXPECT_EQ(result.as<int32_t>(1), 1);
  EXPECT_EQ(result.as<int64_t>(2), 1);
  cnx.execute("DROP TABLE tmp");

}

TEST(result_sync, char_types) {

  Connection cnx;
  cnx.connect();

  EXPECT_STREQ("hello     ", cnx.execute("SELECT CAST('hello' AS CHAR(10))").as<std::string>(0).c_str());
  EXPECT_STREQ("world", cnx.execute("SELECT CAST('world' AS VARCHAR(10))").as<std::string>(0).c_str());
  EXPECT_STREQ("hello world", cnx.execute("SELECT CAST('hello world' AS TEXT)").as<std::string>(0).c_str());
  EXPECT_EQ('X', cnx.execute("SELECT CAST('X' AS \"char\")").as<char>(0));
  EXPECT_STREQ("name", cnx.execute("SELECT CAST('name' AS NAME)").as<std::string>(0).c_str());

}

TEST(result_sync, utf8) {

  Connection cnx;
  cnx.connect();

  EXPECT_STREQ(u8"Günter", cnx.execute("SELECT 'Günter'").as<std::string>(0).c_str());
  EXPECT_STREQ(u8"メインページ", cnx.execute("SELECT 'メインページ'").as<std::string>(0).c_str());

}

TEST(result_sync, date_time_types) {

  Connection cnx;
  cnx.connect();

  EXPECT_EQ(0, cnx.execute("SELECT TIMESTAMP WITH TIME ZONE '1970-01-01 00:00:00+00'").as<timestamptz_t>(0));
  EXPECT_EQ(600123, cnx.execute("SELECT TIMESTAMP WITH TIME ZONE '1970-01-01 00:00:00.600123+00'").as<timestamptz_t>(0));
  EXPECT_EQ(0, cnx.execute("SELECT DATE '1970-01-01'").as<date_t>(0));
  EXPECT_EQ(1451606400, cnx.execute("SELECT DATE '2016-01-01'").as<date_t>(0));

  auto timetz = cnx.execute("SELECT TIME WITH TIME ZONE '00:00:01.000001-07'").as<timetz_t>(0);
  EXPECT_EQ(1000001, timetz.time);
  EXPECT_EQ(7*3600, timetz.offset);

  EXPECT_EQ(39602000101, cnx.execute("SELECT TIME '11:00:02.000101'").as<db::postgres::time_t>(0));

  auto interval = cnx.execute("SELECT INTERVAL '3 months 7 days 2:03:04'").as<interval_t>(0);
  EXPECT_EQ(7384000000, interval.time);
  EXPECT_EQ(7, interval.days);
  EXPECT_EQ(3, interval.months);

  cnx.execute("set timezone TO 'GMT'");
  EXPECT_EQ(cnx.execute("SELECT TIMESTAMP '1970-01-01 00:00:00.600123'").as<timestamp_t>(0), 600123);

}

TEST(result_sync, bytea_type) {

  Connection cnx;
  cnx.connect();

  std::vector<uint8_t> expected;
  expected.push_back(0xDE);
  expected.push_back(0xAD);
  expected.push_back(0xBE);
  expected.push_back(0xEF);

  std::vector<uint8_t> actual = cnx.execute("SELECT CAST(E'\\\\xDEADBEEF' AS BYTEA)").as<std::vector<uint8_t>>(0);
  EXPECT_TRUE(expected.size() == actual.size() && std::equal(actual.begin(), actual.end(), expected.begin()));

}

TEST(result_sync, arrays) {

  Connection cnx;
  cnx.connect();

  TEST_VECTOR(cnx.execute("SELECT array[true,false,true]")
              .asArray<bool>(0), ARRAY({bool(true), bool(false), bool(true)}), 3, bool);
    EXPECT_EQ(expected, actual.value);
    EXPECT_FALSE(actual.isNull);
  TEST_VECTOR_END;

  TEST_VECTOR(cnx.execute("SELECT array[1::smallint,2::smallint,3::smallint]")
              .asArray<int16_t>(0), ARRAY({1, 2, 3}), 3, int16_t);
    EXPECT_EQ(expected, actual.value);
    EXPECT_FALSE(actual.isNull);
  TEST_VECTOR_END;

  TEST_VECTOR(cnx.execute("SELECT ARRAY[4,5,6]")
              .asArray<int32_t>(0), ARRAY({4, 5, 6}), 3, int32_t);
    EXPECT_EQ(expected, actual.value);
    EXPECT_FALSE(actual.isNull);
  TEST_VECTOR_END;

  TEST_VECTOR(cnx.execute("SELECT ARRAY[7::bigint,8::bigint,9::bigint,10::bigint]")
              .asArray<int64_t>(0), ARRAY({7, 8, 9, 10}), 4, int64_t);
    EXPECT_EQ(expected, actual.value);
    EXPECT_FALSE(actual.isNull);
  TEST_VECTOR_END;

  TEST_VECTOR(cnx.execute("SELECT array[1.89::real,-9.998::real,3::real]")
              .asArray<float>(0), ARRAY({1.89f, -9.998f, 3.f}), 3, float);
    EXPECT_FLOAT_EQ(expected, actual.value);
    EXPECT_FALSE(actual.isNull);
  TEST_VECTOR_END;

  TEST_VECTOR(cnx.execute("SELECT array[7.123::double precision,0.98::double precision]")
              .asArray<double>(0), ARRAY({7.123, 0.98}), 2, double);
    EXPECT_DOUBLE_EQ(expected, actual.value);
    EXPECT_FALSE(actual.isNull);
  TEST_VECTOR_END;

  TEST_VECTOR(cnx.execute("SELECT ARRAY['hello', 'world']")
              .asArray<std::string>(0), ARRAY({"hello", "world"}), 2, std::string);
    EXPECT_STREQ(expected.c_str(), actual.value.c_str());
    EXPECT_FALSE(actual.isNull);
  TEST_VECTOR_END;

  TEST_VECTOR(cnx.execute("select ARRAY['01:12:01.000001-07'::timetz, '00:00:00.000000-00'::timetz]")
              .asArray<timetz_t>(0), ARRAY({{4321000001, 25200}, {0, 0}}), 2, timetz_t);
  EXPECT_EQ(expected.time, actual.value.time);
  EXPECT_EQ(expected.offset, actual.value.offset);
  TEST_VECTOR_END;

  {
    auto array = cnx.execute("SELECT ARRAY['hello', 'world']").asArray<std::string>(0);
    std::string hello = array[0];
    std::string world = array[1];
    EXPECT_STREQ("hello", hello.c_str());
    EXPECT_STREQ("world", world.c_str());
  }

  {
    auto array = cnx.execute("SELECT ARRAY[7::bigint,8::bigint,9::bigint,10::bigint]").asArray<int64_t>(0);
    int64_t v1 = array[0];
    int64_t v2 = array[1];
    EXPECT_EQ(7, v1);
    EXPECT_EQ(8, v2);
  }

}

TEST(result_sync, null_values) {

  Connection cnx;
  cnx.connect();

  EXPECT_TRUE(cnx.execute("SELECT NULL::bigint").isNull(0));
  EXPECT_FALSE(cnx.execute("SELECT NULL::bool").as<bool>(0));
  EXPECT_EQ(0, cnx.execute("SELECT NULL::smallint").as<int16_t>(0));
  EXPECT_EQ(0, cnx.execute("SELECT NULL::integer").as<int32_t>(0));
  EXPECT_EQ(0., cnx.execute("SELECT NULL::float4").as<float>(0));
  EXPECT_EQ(0., cnx.execute("SELECT NULL::float8").as<double>(0));
  EXPECT_EQ(0, cnx.execute("SELECT NULL::bytea").as<std::vector<uint8_t>>(0).size());
  EXPECT_EQ(0, cnx.execute("SELECT NULL::date").as<date_t>(0));
  EXPECT_EQ(0, cnx.execute("SELECT NULL::timestamptz").as<timestamptz_t>(0));
  EXPECT_EQ(0, cnx.execute("SELECT NULL::timestamp").as<timestamp_t>(0));
  EXPECT_EQ(0, cnx.execute("SELECT NULL::time").as<db::postgres::time_t>(0));
  auto timetz = cnx.execute("SELECT NULL::timetz").as<db::postgres::timetz_t>(0);
  EXPECT_EQ(0, timetz.time);
  EXPECT_EQ(0, timetz.offset);
  auto interval = cnx.execute("SELECT NULL::interval").as<db::postgres::interval_t>(0);
  EXPECT_EQ(0, interval.time);
  EXPECT_EQ(0, interval.days);
  EXPECT_EQ(0, interval.months);

}

TEST(result_sync, arrays_null_values) {

  Connection cnx;
  cnx.connect();

  TEST_VECTOR(cnx.execute("SELECT array[1, null, 3]")
              .asArray<int32_t>(0), ARRAY({false, true, false}), 3, bool);
    EXPECT_EQ(expected, actual.isNull);
  TEST_VECTOR_END;

  TEST_VECTOR(cnx.execute("SELECT ARRAY[null::bigint, null::bigint, 3::bigint, 4::bigint]")
              .asArray<int64_t>(0), ARRAY({true, true, false, false}), 4, bool);
    EXPECT_EQ(expected, actual.isNull);
  TEST_VECTOR_END;

  TEST_VECTOR(cnx.execute("SELECT ARRAY['hello', null, 'world']")
              .asArray<std::string>(0), ARRAY({false, true, false}), 3, bool);
    EXPECT_EQ(expected, actual.isNull);
  TEST_VECTOR_END;
}

TEST(result_sync, column_name) {

  Connection cnx;
  cnx.connect();

  auto &result = cnx.execute("SELECT 0 AS c1, 1, 2 AS \"Günter\", 3 AS \"メインページ\"");
  EXPECT_STREQ("c1", result.columnName(0));
  EXPECT_STREQ("?column?", result.columnName(1));
  EXPECT_STREQ(u8"Günter", result.columnName(2));
  EXPECT_STREQ(u8"メインページ", result.columnName(3));

}
