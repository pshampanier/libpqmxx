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

TEST(result, sync_datatypes) {

  Connection cnx;
  cnx.connect("postgresql://postgres@localhost");

  EXPECT_EQ(cnx.execute("SELECT true").result().get<bool>(0), true);
  EXPECT_EQ(cnx.execute("SELECT false").result().get<bool>(0), false);
  EXPECT_EQ(cnx.execute("SELECT CAST(32767 AS SMALLINT)").result().get<int16_t>(0), 32767);
  EXPECT_EQ(cnx.execute("SELECT 2147483647").result().get<int32_t>(0), 2147483647);
  EXPECT_EQ(cnx.execute("SELECT 9223372036854775807").result().get<int64_t>(0), 9223372036854775807);
  EXPECT_STREQ(cnx.execute("SELECT CAST('hello' AS CHAR(10))").result().get<std::string>(0).c_str(), "hello     ");
  EXPECT_STREQ(cnx.execute("SELECT CAST('hello' AS VARCHAR(10))").result().get<std::string>(0).c_str(), "hello");

  return;

  // cnx.execute("SELECT $1", 23);
}
