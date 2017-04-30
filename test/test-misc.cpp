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
#include <gtest/gtest.h>

using namespace db::postgres;
using namespace db::postgres::literals;

TEST(misc, cancel) {

  Connection cnx;
  cnx.connect();

  auto result = cnx.execute("SELECT generate_series(1, 10000)");
  int rownum = 0;

  for (auto &row: result) {
    rownum = row.num();
    if (row.num() == 100) {
      cnx.cancel();
      break;
    }
  }

}

TEST(misc, notice) {

  std::string notice;
  Connection cnx;
  cnx.connect().notice([&notice](const char *message) {
    notice = message;
  }).execute("DROP TABLE IF EXISTS __no_way_it_exists_00001"_x);
  EXPECT_TRUE(notice.find("__no_way_it_exists_00001") != std::string::npos);

  // Disabling the notice, the previous handler should no longer be called and
  // the notice string should stay untouched.
  cnx.notice(nullptr).execute("DROP TABLE IF EXISTS __no_way_it_exists_00002"_x);
  EXPECT_TRUE(notice.find("__no_way_it_exists_00001") != std::string::npos);

}