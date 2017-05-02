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
using namespace libpqmxx::literals;
namespace async = libpqmxx::async_boost;

TEST(boost, connect) {

  ::boost::asio::io_service ioService;
  auto cnx = std::make_shared<async::Connection>(ioService);
  cnx->connect(nullptr, [cnx](std::exception_ptr &eptr) {
    EXPECT_TRUE(eptr == nullptr);
    cnx->close();
  });
  ioService.run();

}

TEST(boost, connect_invalid_host) {

  ::boost::asio::io_service ioService;
  auto cnx = std::make_shared<async::Connection>(ioService);
  cnx->connect("postgresql://foo@63e39014-1897-4143-af19-1f44148acc7f", [](std::exception_ptr &eptr) {
    try {
      std::rethrow_exception(eptr);
    }
    catch (connection_error &e) {
      EXPECT_EQ(error_code::connection_failure, e.code());
    }
    catch (...) {
      FAIL();
    }
  });
  ioService.run();

}

TEST(boost, connect_refused) {

  ::boost::asio::io_service ioService;
  auto cnx = std::make_shared<async::Connection>(ioService);
  cnx->connect("postgresql://ci-test@127.0.0.1:1", [](std::exception_ptr &eptr) {
    try {
      std::rethrow_exception(eptr);
    }
    catch (connection_error &e) {
      EXPECT_EQ(error_code::connection_failure, e.code());
    }
    catch (...) {
      FAIL();
    }
  });
  ioService.run();

}

TEST(boost, singleton) {

  ::boost::asio::io_service ioService;

  {
    int32_t actual = 0;

    auto cnx = std::make_shared<async::Connection>(ioService);
    cnx->connect(nullptr, [cnx, &actual](std::exception_ptr &) {
      cnx->execute("SELECT $1", 42).done([cnx, &actual](Result &result) {
        actual = result.as<int32_t>(0);
        cnx->close();
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

    auto cnx = std::make_shared<async::Connection>(ioService);
    cnx->connect(nullptr, [cnx, &what](std::exception_ptr &) {
      cnx->execute("SELECT $1", 42).done([](Result &) {
        throw std::runtime_error("test");
      }).error([cnx, &what](std::exception_ptr &e) {
        try {
          std::rethrow_exception(e);
        }
        catch (const std::runtime_error &e) {
          what = e.what();
          cnx->close();
        }
      });
    });

    ioService.run();
    EXPECT_STREQ("test", what.c_str());
  }

  { //
    // Error of the execute call
    //
    bool done = false, error = false;
    ioService.reset();

    auto cnx = std::make_shared<async::Connection>(ioService);
    cnx->connect(nullptr, [cnx, &done, &error](std::exception_ptr &) {
      cnx->execute("SELECT $1 FROM does_not_exists", 42).done([&done](Result &) {
        done = true;
      }).error([cnx, &error](std::exception_ptr &e) {
        EXPECT_THROW(std::rethrow_exception(e), libpqmxx::error);
        error = true;
        cnx->close();
      });
    });

    ioService.run();
    EXPECT_FALSE(done);
    EXPECT_TRUE(error);
  }

}

TEST(boost, iterator) {

  ::boost::asio::io_service ioService;
  int64_t sum = 0, count = 0;
  auto cnx = std::make_shared<async::Connection>(ioService);
  cnx->connect(nullptr, [cnx, &sum, &count](std::exception_ptr &) {
    cnx->execute("SELECT generate_series(1, 100000)").done([cnx, &sum, &count](Result &result) {
      for (auto &row: result) {
        sum += row.as<int32_t>(0);
      }
      count = result.count();
      cnx->close();
    });
  });

  ioService.run();
  EXPECT_EQ(5000050000, sum);
  EXPECT_EQ(100000, count);
}

TEST(boost, empty_result) {

  ::boost::asio::io_service ioService;
  int64_t count = 1;
  auto cnx = std::make_shared<async::Connection>(ioService);
  cnx->connect(nullptr, [cnx, &count](std::exception_ptr &) {
    cnx->execute("SELECT 42 WHERE 1 = 2").done([cnx, &count](Result &result) {
      count = result.count();
      cnx->close();
    });
  });

  ioService.run();
  EXPECT_EQ(0, count);

}

TEST(boost, execute_error) {
  
  ::boost::asio::io_service ioService;
  bool done = false, error = false, always = false;
  auto cnx = std::make_shared<async::Connection>(ioService);
  cnx->connect(nullptr, [&done, &error, &always, cnx](std::exception_ptr &) {
    cnx->execute("SELECT 42/0").done([&done](Result &) {
      done = true;
    }).error([&error](std::exception_ptr &) {
      error = true;
    }).always([&always, cnx](Connection &) {
      always = true;
      cnx->close();
    });
  });
  
  ioService.run();
  EXPECT_FALSE(done);
  EXPECT_TRUE(error);
  EXPECT_TRUE(always);
  
}

TEST(boost, synchronous_execute_error) {

  ::boost::asio::io_service ioService;
  bool done = false, error = false, always = false;
  auto cnx = std::make_shared<async::Connection>(ioService);
  cnx->connect(nullptr, [&done, &error, &always, cnx](std::exception_ptr &) {
    cnx->execute(nullptr).done([&done](Result &) {
      done = true;
    }).error([&error](std::exception_ptr &) {
      error = true;
    }).always([&always, cnx](Connection &) {
      always = true;
      cnx->close();
    });
  });

  ioService.run();
  EXPECT_FALSE(done);
  EXPECT_TRUE(error);
  EXPECT_TRUE(always);

}

TEST(boost, client_encoding) {

  ::boost::asio::io_service ioService;
  Settings settings;
  settings.encoding = "__invalid__";
  auto cnx = std::make_shared<async::Connection>(ioService, settings);
  cnx->connect(nullptr, [cnx](std::exception_ptr &eptr) {
    try {
      std::rethrow_exception(eptr);
    }
    catch (connection_error &e) {
      EXPECT_EQ(error_code::connection_failure, e.code());
    }
    catch (...) {
      FAIL();
    }
  });

  ioService.run();

}

TEST(boost, statement) {
  
  ::boost::asio::io_service ioService;
  int64_t inserts = 0, deletes = 0;
  auto cnx = std::make_shared<async::Connection>(ioService);
  cnx->connect(nullptr, [cnx, &inserts, &deletes](std::exception_ptr &) {
    cnx->execute(R"SQL(
                 
      CREATE TEMPORARY TABLE boost_statement (id INTEGER NOT NULL);
      INSERT INTO boost_statement VALUES (10001), (10002), (10003);
      DELETE FROM boost_statement WHERE id > 10001;
                 
    )SQL"_x).done([cnx, &inserts, &deletes](Result &result) {
    
      result.done([cnx, &inserts, &deletes](Result &result) {
        
        inserts = result.count();
        result.done([cnx, &deletes](Result &result) {
          deletes = result.count();
          cnx->close();
        });
        
      });
    
    });
  });
  
  ioService.run();
  EXPECT_EQ(3, inserts);
  EXPECT_EQ(2, deletes);

}
