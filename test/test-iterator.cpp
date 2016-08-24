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

TEST(iterator, sync_no_row) {

  Connection cnx;
  cnx.connect();

  int32_t actual = 0;

  auto &result = cnx.execute("SELECT 1 WHERE 1=2");
  for (auto &row: result) {
    actual += row.as<int32_t>(0);
  }

  EXPECT_EQ(actual, 0);

}

TEST(iterator, sync_one_row) {

  Connection cnx;
  cnx.connect();

  int32_t actual = 0;

  auto &result = cnx.execute("SELECT 42");
  for (auto &row: result) {
    actual += row.as<int32_t>(0);
  }

  EXPECT_EQ(actual, 42);

}

TEST(iterator, sync_multiple_rows) {

  Connection cnx;
  cnx.connect();

  int32_t actual = 0;

  auto &result = cnx.execute("SELECT generate_series(1, 3)");
  for (auto &row: result) {
    actual += row.as<int32_t>(0);
  }

  EXPECT_EQ(actual, 6);

}

TEST(iterator, sync_rownum) {

  Connection cnx;
  cnx.connect();

  int32_t actual = 0;

  auto &result = cnx.execute("SELECT generate_series(1, 3)");
  for (auto &row: result) {
    actual += row.num();
  }

  // rownum should reset for the next query
  auto &result2 = cnx.execute("SELECT generate_series(1, 3)");
  for (auto &row: result2) {
    actual += row.num();
  }

  EXPECT_EQ(actual, 12);

}
