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

TEST(params_sync, datatypes) {

  Connection cnx;
  cnx.connect("postgresql://postgres@localhost");

  EXPECT_EQ(cnx.execute("SELECT $1::smallint", int16_t(32767)).result().get<int16_t>(0), 32767);
  EXPECT_EQ(cnx.execute("SELECT $1", 2147483647).result().get<int32_t>(0), 2147483647);
  EXPECT_EQ(cnx.execute("SELECT $1", int64_t(9223372036854775807)).result().get<int64_t>(0), 9223372036854775807);
  EXPECT_FLOAT_EQ(cnx.execute("SELECT $1::real", float(0.45567)).result().get<float>(0), 0.45567);
  EXPECT_FLOAT_EQ(cnx.execute("SELECT $1::double precision", double(0.45567)).result().get<double>(0), 0.45567);
  EXPECT_EQ(cnx.execute("SELECT $1", true).result().get<bool>(0), true);
  EXPECT_EQ(cnx.execute("SELECT $1", false).result().get<bool>(0), false);
  EXPECT_STREQ(cnx.execute("SELECT $1", "hello").result().get<std::string>(0).c_str(), "hello");
  EXPECT_STREQ(cnx.execute("SELECT $1", std::string("hello")).result().get<std::string>(0).c_str(), "hello");
  EXPECT_EQ(cnx.execute("SELECT $1", 'X').result().get<char>(0), 'X');

}

TEST(params_sync, utf8) {

  Connection cnx;
  cnx.connect("postgresql://postgres@localhost");

  EXPECT_STREQ(cnx.execute("SELECT $1", u8"Günter").result().get<std::string>(0).c_str(), u8"Günter");
  EXPECT_STREQ(cnx.execute("SELECT $1", u8"メインページ").result().get<std::string>(0).c_str(), u8"メインページ");

}


TEST(param_sync, bytea_type) {

  Connection cnx;
  cnx.connect("postgresql://postgres@localhost");

  std::vector<uint8_t> expected;
  expected.push_back(0xDE);
  expected.push_back(0xAD);
  expected.push_back(0xBE);
  expected.push_back(0xEF);

  std::vector<uint8_t> actual = cnx.execute("SELECT $1::bytea", expected).result().get<std::vector<uint8_t>>(0);
  EXPECT_TRUE(expected.size() == actual.size() && std::equal(actual.begin(), actual.end(), expected.begin()));

}

