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
#include "postgres-boost.h"
#include <gtest/gtest.h>

using namespace libpqmxx;

TEST(sync, connect) {

  Connection cnx;

  // OK
  EXPECT_NO_THROW(cnx.connect().close());

  // DNS RESOLUTION ERROR
  try {
    cnx.connect("postgresql://foo@63e39014-1897-4143-af19-1f44148acc7f");
  }
  catch (connection_error err) {
    EXPECT_EQ(error_code::connection_failure, err.code());
  }
  catch (...) {
    FAIL();
  }

  // CONNECTION REFUSED
  try {
    cnx.connect("postgresql://foo@127.0.0.1:1");
  }
  catch (connection_error err) {
    EXPECT_EQ(error_code::connection_failure, err.code());
  }
  catch (...) {
    FAIL();
  }

}
