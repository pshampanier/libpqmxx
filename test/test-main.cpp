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
#include <cstdlib>
#include <iostream>
#include <stdlib.h>

#ifdef WIN32
  #pragma warning(disable : 4996)
  #define setenv(name, value, overwrite) _putenv_s(name, value);
#endif

const char *DEFAULT_PGHOST = "localhost";
const char *DEFAULT_PGUSER = "ci-test";
const char *DEFAULT_PGDATABASE = "ci-test";

const char *init(const char *name, const char *defValue) {
  const char *value = std::getenv(name);
  if (value == nullptr) {
    setenv(name, defValue, 1);
    value = defValue;
  }
  return value;
}

int main(int argc, char** argv) {

  const char *pghost = init("PGHOST", DEFAULT_PGHOST);
  const char *pguser = init("PGUSER", DEFAULT_PGUSER);
  const char *pgdatabase = init("PGDATABASE", DEFAULT_PGDATABASE);

  std::cout << "Using "<< "PGHOST=" << pghost << " PGUSER=" << pguser
    << " PGDATABASE=" << pgdatabase << std::endl;

  // This allows the user to override the flag on the command line.
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
