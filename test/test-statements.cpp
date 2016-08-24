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

TEST(result_sync, statements) {

  Connection cnx;
  cnx.connect();

  cnx.execute(R"SQL(

    DROP TABLE IF EXISTS titles;
    DROP TABLE IF EXISTS employees;
    DROP TYPE IF EXISTS GENDER;

  )SQL");

  cnx.execute(R"SQL(

    CREATE TYPE GENDER AS ENUM ('M', 'F');

    CREATE TABLE employees (
      emp_no      INTEGER         NOT NULL,
      birth_date  DATE            NOT NULL,
      first_name  VARCHAR(14)     NOT NULL,
      last_name   VARCHAR(16)     NOT NULL,
      gender      GENDER          NOT NULL,
      hire_date   DATE            NOT NULL,
      PRIMARY KEY (emp_no)
    );

    CREATE TABLE titles (
        emp_no      INT             NOT NULL,
        title       VARCHAR(50)     NOT NULL,
        from_date   DATE            NOT NULL,
        to_date     DATE,
        FOREIGN KEY (emp_no) REFERENCES employees (emp_no) ON DELETE CASCADE,
        PRIMARY KEY (emp_no,title, from_date)
    );

  )SQL");

  EXPECT_EQ(20, cnx.execute(R"SQL(

    INSERT INTO employees VALUES
      (10001,'1953-09-02','Georgi','Facello','M','1986-06-26'),
      (10002,'1964-06-02','Bezalel','Simmel','F','1985-11-21'),
      (10003,'1959-12-03','Parto','Bamford','M','1986-08-28'),
      (10004,'1954-05-01','Chirstian','Koblick','M','1986-12-01'),
      (10005,'1955-01-21','Kyoichi','Maliniak','M','1989-09-12'),
      (10006,'1953-04-20','Anneke','Preusig','F','1989-06-02'),
      (10007,'1957-05-23','Tzvetan','Zielinski','F','1989-02-10'),
      (10008,'1958-02-19','Saniya','Kalloufi','M','1994-09-15'),
      (10009,'1952-04-19','Sumant','Peac','F','1985-02-18'),
      (10010,'1963-06-01','Duangkaew','Piveteau','F','1989-08-24'),
      (10011,'1953-11-07','Mary','Sluis','F','1990-01-22'),
      (10012,'1960-10-04','Patricio','Bridgland','M','1992-12-18'),
      (10013,'1963-06-07','Eberhardt','Terkki','M','1985-10-20'),
      (10014,'1956-02-12','Berni','Genin','M','1987-03-11'),
      (10015,'1959-08-19','Guoxiang','Nooteboom','M','1987-07-02'),
      (10016,'1961-05-02','Kazuhito','Cappelletti','M','1995-01-27'),
      (10017,'1958-07-06','Cristinel','Bouloucos','F','1993-08-03'),
      (10018,'1954-06-19','Kazuhide','Peha','F','1987-04-03'),
      (10019,'1953-01-23','Lillian','Haddadi','M','1999-04-30'),
      (10020,'1952-12-24','Mayuko','Warwick','M','1991-01-26');

  )SQL").count());

  EXPECT_EQ(28, cnx.execute(R"SQL(

    INSERT INTO titles VALUES
      (10001,'Senior Engineer','1986-06-26',NULL),
      (10002,'Staff','1996-08-03',NULL),
      (10003,'Senior Engineer','1995-12-03',NULL),
      (10004,'Engineer','1986-12-01','1995-12-01'),
      (10004,'Senior Engineer','1995-12-01',NULL),
      (10005,'Senior Staff','1996-09-12',NULL),
      (10005,'Staff','1989-09-12','1996-09-12'),
      (10006,'Senior Engineer','1990-08-05',NULL),
      (10007,'Senior Staff','1996-02-11',NULL),
      (10007,'Staff','1989-02-10','1996-02-11'),
      (10008,'Assistant Engineer','1998-03-11','2000-07-31'),
      (10009,'Assistant Engineer','1985-02-18','1990-02-18'),
      (10009,'Engineer','1990-02-18','1995-02-18'),
      (10009,'Senior Engineer','1995-02-18',NULL),
      (10010,'Engineer','1996-11-24',NULL),
      (10011,'Staff','1990-01-22','1996-11-09'),
      (10012,'Engineer','1992-12-18','2000-12-18'),
      (10012,'Senior Engineer','2000-12-18',NULL),
      (10013,'Senior Staff','1985-10-20',NULL),
      (10014,'Engineer','1993-12-29',NULL),
      (10015,'Senior Staff','1992-09-19','1993-08-22'),
      (10016,'Staff','1998-02-11',NULL),
      (10017,'Senior Staff','2000-08-03',NULL),
      (10017,'Staff','1993-08-03','2000-08-03'),
      (10018,'Engineer','1987-04-03','1995-04-03'),
      (10018,'Senior Engineer','1995-04-03',NULL),
      (10019,'Staff','1999-04-30',NULL),
      (10020,'Engineer','1997-12-30',NULL);

  )SQL").count());

  auto &genders = cnx.execute(R"SQL(

    SELECT e.emp_no, gender
      FROM employees e JOIN titles t on (e.emp_no=t.emp_no)
     WHERE t.to_date IS NULL ORDER BY t.from_date DESC LIMIT 3

  )SQL");

  char gender;
  int32_t males = 0, females = 0, ids = 0;
  for (auto &row: genders) {
    ids += row.as<std::int32_t>(0);
    gender = row.as<char>(1);
    if (gender == 'M') {
      males++;
    }
    else if (gender == 'F') {
      females++;
    }
  }

  EXPECT_EQ(30048, ids);
  EXPECT_EQ(2, males);
  EXPECT_EQ(1, females);

  EXPECT_THROW(cnx.execute("INSERT INTO titles VALUES (10021,'Technique Leader','1988-02-10','2002-07-15')"), ExecutionException);

  cnx.execute("UPDATE titles SET to_date=$1::date WHERE emp_no=$2", "1988-02-10", 10020);
  cnx.execute("INSERT INTO titles VALUES ($1, $2, $3::date, $4)",
              10020, "Technique Leader", "1988-02-10", nullptr);

  EXPECT_EQ(883440000, cnx.execute(R"SQL(
    SELECT from_date FROM titles WHERE  to_date=$1::date AND emp_no=$2
  )SQL", "1988-02-10", 10020).as<date_t>(0));

  // Cleanup
  cnx.execute(R"SQL(

    DROP TABLE IF EXISTS titles;
    DROP TABLE IF EXISTS employees;
    DROP TYPE IF EXISTS GENDER;

  )SQL");

}

TEST(misc, cancel) {

  Connection cnx;
  cnx.connect();

  auto &result = cnx.execute("SELECT generate_series(1, 10000)");
  int rownum = 0;

  for (auto &row: result) {
    rownum = row.num();
    if (row.num() == 100) {
      cnx.cancel();
      break;
    }
  }

}

TEST(misc, is_single_statement) {

  //
  // Dollar-Quoted String Constants
  //

  EXPECT_TRUE(isSingleStatement("SELECT $$Dianne's ;horse$$;"));
  EXPECT_FALSE(isSingleStatement("SELECT $$1;'\n$$; SELECT $$Dianne's ;horse$$;"));
  EXPECT_TRUE(isSingleStatement("SELECT $ab$Dianne's ;horse$ab$;"));
  EXPECT_FALSE(isSingleStatement("SELECT $ab$1;'\n$ab$; SELECT $cd$Dianne's ;horse$cd$;"));

  EXPECT_TRUE(isSingleStatement(R"SQL(

    CREATE FUNCTION fn() RETURNS INTEGER AS $function$
    BEGIN
        RETURN ($1 ~ $q$[\t\r\n\v\\]$q$);
    END;
    $function$ language plpgsql;

  )SQL"));

  EXPECT_FALSE(isSingleStatement(R"SQL(

    CREATE FUNCTION fn() RETURNS INTEGER AS $function$
    BEGIN
        RETURN ($1 ~ $q$[\t\r\n\v\\]$q$);
    END;
    $function$ language plpgsql;

    SELECT fn();

  )SQL"));

  EXPECT_FALSE(isSingleStatement(R"SQL(

    DROP TYPE IF EXISTS GENDER;
    CREATE TYPE GENDER AS ENUM ('M', 'F');

  )SQL"));

  EXPECT_FALSE(isSingleStatement(R"SQL(

    CREATE TABLE employees (
      emp_no      INTEGER         NOT NULL,
      birth_date  DATE            NOT NULL,
      first_name  VARCHAR(14)     NOT NULL,
      last_name   VARCHAR(16)     NOT NULL,
      gender      GENDER          NOT NULL,
      hire_date   DATE            NOT NULL,
      PRIMARY KEY (emp_no)
    );

    CREATE TABLE titles (
        emp_no      INT             NOT NULL,
        title       VARCHAR(50)     NOT NULL,
        from_date   DATE            NOT NULL,
        to_date     DATE,
        FOREIGN KEY (emp_no) REFERENCES employees (emp_no) ON DELETE CASCADE,
        PRIMARY KEY (emp_no,title, from_date)
    );

  )SQL"));

  //
  // Comments
  //

  EXPECT_TRUE(isSingleStatement(R"SQL(

    --
    -- This is a comment;
    --
    SELECT 1;
    -- $$hello$$;'ok'"OK"

  )SQL"));

  EXPECT_FALSE(isSingleStatement(R"SQL(

    -- This is a comment;
    SELECT 1;
    -- This is a comment;
    SELECT 2;

  )SQL"));

  //
  // C-style block comments
  //

  EXPECT_TRUE(isSingleStatement(R"SQL(

    /* comment */
    SELECT 1;

  )SQL"));

  EXPECT_TRUE(isSingleStatement(R"SQL(

    SELECT /* comment */ 1;

  )SQL"));

  EXPECT_TRUE(isSingleStatement(R"SQL(

    /* multiline comment
     * with nesting: /* nested block comment */
     */
    SELECT 1;

  )SQL"));

  EXPECT_FALSE(isSingleStatement(R"SQL(

   /* multiline comment
    * with nesting: /* nested block comment */
    */
   SELECT /* comment */ 1;
   SELECT /* comment */ 2;

  )SQL"));

}
