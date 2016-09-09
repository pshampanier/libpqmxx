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
#include "postgres-boost.h"

using namespace db::postgres;

TEST(async, connect) {

  ::boost::asio::io_service ioService;

  {
    auto cnx = std::make_shared<db::postgres::boost::Connection>(ioService);
    cnx->connect("postgresql://ci-test@localhost").done([]() {
      SUCCEED();
    }).error([](std::exception_ptr reason) {
      FAIL();
    });
    ioService.run();
  }

  {
    ioService.reset();
    auto cnx = std::make_shared<db::postgres::boost::Connection>(ioService);
    cnx->connect("postgresql://invalid-db@localhost").done([]() {
      FAIL();
    }).error([](std::exception_ptr reason) {
      EXPECT_THROW(std::rethrow_exception(reason), db::postgres::ConnectionException);
    });
    ioService.run();
  }

  {
    ioService.reset();
    auto cnx = std::make_shared<db::postgres::boost::Connection>(ioService);
    cnx->connect("postgresql://ci-test@invalid_host").done([]() {
      FAIL();
    }).error([](std::exception_ptr reason) {
      EXPECT_THROW(std::rethrow_exception(reason), ConnectionException);
    });
    ioService.run();
  }

}

TEST(async, once) {

  ::boost::asio::io_service ioService;

  {
    int32_t actual = 0;

    auto cnx = std::make_shared<db::postgres::boost::Connection>(ioService);
    cnx->connect("postgresql://ci-test@localhost").done([&cnx, &actual]() {
      cnx->execute("SELECT $1", 42).once([&actual](const Row &row) {
        actual = row.as<int32_t>(0);
      });
    });

    ioService.run();
    EXPECT_EQ(42, actual);
  }

  { //
    // Error inside the iterator once
    //
    std::string what;
    ioService.reset();

    auto cnx = std::make_shared<db::postgres::boost::Connection>(ioService);
    cnx->connect("postgresql://ci-test@localhost").done([&cnx, &what]() {
      cnx->execute("SELECT $1", 42).once([](const Row &row) {
        throw std::runtime_error("test");
      }).error([&what](std::exception_ptr e) {
        try {
          std::rethrow_exception(e);
        }
        catch (const std::runtime_error &e) {
          what = e.what();
        }
      });
    });

    ioService.run();
    EXPECT_STREQ("test", what.c_str());
  }

  { //
    // Error of the execute call
    //
    bool error = false;
    ioService.reset();

    auto cnx = std::make_shared<db::postgres::boost::Connection>(ioService);
    cnx->connect("postgresql://ci-test@localhost").done([&cnx, &error]() {
      cnx->execute("SELECT $1 FROM does_not_exists", 42).done([](int64_t) {
      }).error([&error](std::exception_ptr e) {
        EXPECT_THROW(std::rethrow_exception(e), ExecutionException);
        error = true;
      });
    });

    ioService.run();
    EXPECT_TRUE(error);
  }

}

TEST(async, each) {

  ::boost::asio::io_service ioService;

  int32_t actual = 0;

  auto cnx = std::make_shared<db::postgres::boost::Connection>(ioService);
  cnx->connect("postgresql://ci-test@localhost").done([&cnx, &actual]() {
    cnx->execute("SELECT generate_series(1, 10000)").each([&actual](const Row &row) -> bool {
      actual += row.as<int32_t>(0);
      return true;
    });
  });

  ioService.run();
  EXPECT_EQ(50005000, actual);
}

