# <img src="help/assets/libpqmxx-logo.png" height="60"/>


> A simple PostgreSQL client library written in Modern C++.

[![License][license-mit-img]][license-mit]

libpq**m**xx is an unofficial C++ 11 client API for PostgreSQL.

The aim of this library is to provide an API very simple to use without any 
compromise on performances. Modern C++ features such as variadic templates are 
used to make the programing interface slick, very easy to use and the code to read.

* **Automatic detection of the PostgresSQL datatype from C++ parameter's type**.

    ```cpp
    cnx.execute("SELECT from_date FROM titles WHERE emp_no=$1", 10020);
    ```
    
    Because `10020` is an `int` in C++, the library will bind this parameter to a PostgreSQL `integer`.  
    
* **One single method to get a column value from a row independently of the data type**.

    ```cpp
    int32_t emp_no = row.get<int32_t>(0);
    std::string title = row.get<std::string>(1);
    ```
    
* **Use of the range-based for statement to iterate through the result.**

    ```cpp
    auto &employees = cnx.execute("SELECT emp_no, first_name || ' ' || last_name FROM employees");
    for (auto &employee: employees) {
      std::cout << row.get<int32_t>(0) << row.get<std::string>(1) << std::endl;
    }
    ```  
* **Results with only one row can be accessed without using the iterator**:

    ```cpp
    auto &employee = cnx.execute("SELECT last_name FROM employees WHERE emp_no=$1", 10001);
    std::cout << employee.get<std::string>(0) << std::endl;
    ```
    
* **`nullptr` can be used to set a null value**.

    ```cpp
    cnx.execute("INSERT INTO titles VALUES ($1, $2, $3::date, $4)",
                10020, "Technique Leader", "1988-02-10", nullptr);
    ```

The online API documentation is available on [gitbook](https://pshampanier.gitbooks.io/libpqmxx/content/).  
If you are looking for the official C++ client library fro PostgreSQL, please visit [pqxx.org](http://pqxx.org).

```cpp
#include "postgres-connection.h"
#include "postgres-exceptions.h"

#include <iostream>

using namespace db::postgres;

int main() {

  Connection cnx;
  try {
    cnx.connect("postgresql://ci-test@localhost");

    cnx.execute(R"SQL(

      DROP TABLE IF EXISTS employees;

      CREATE TABLE employees (
        emp_no      INTEGER         NOT NULL,
        birth_date  DATE            NOT NULL,
        first_name  VARCHAR(14)     NOT NULL,
        last_name   VARCHAR(16)     NOT NULL,
        gender      "char"          NOT NULL,
        hire_date   DATE            NOT NULL,
        PRIMARY KEY (emp_no)
      );

    )SQL");

    std::cout << "Table created." << std::endl;

    int employees = cnx.execute(R"SQL(

      INSERT INTO employees VALUES
        (10001,'1973-09-02','Georgi','Facello','M','2006-06-26'),
        (10002,'1984-06-02','Bezalel','Simmel','F','2005-11-21'),
        (10003,'1979-12-03','Parto','Bamford','M','2006-08-28'),
        (10004,'1974-05-01','Chirstian','Koblick','M','2006-12-01'),
        (10005,'1975-01-21','Kyoichi','Maliniak','M','2009-09-12'),
        (10006,'1973-04-20','Anneke','Preusig','F','2009-06-02'),
        (10007,'1977-05-23','Tzvetan','Zielinski','F','2009-02-10'),
        (10008,'1978-02-19','Saniya','Kalloufi','M','2014-09-15'),
        (10009,'1972-04-19','Sumant','Peac','F','2005-02-18'),
        (10010,'1983-06-01','Duangkaew','Piveteau','F','2009-08-24'),
        (10011,'1973-11-07','Mary','Sluis','F','2010-01-22'),
        (10012,'1980-10-04','Patricio','Bridgland','M','2012-12-18'),
        (10013,'1983-06-07','Eberhardt','Terkki','M','2005-10-20'),
        (10014,'1976-02-12','Berni','Genin','M','2007-03-11'),
        (10015,'1979-08-19','Guoxiang','Nooteboom','M','2007-07-02'),
        (10016,'1981-05-02','Kazuhito','Cappelletti','M','2015-01-27'),
        (10017,'1978-07-06','Cristinel','Bouloucos','F','2013-08-03'),
        (10018,'1984-06-19','Kazuhide','Peha','F','2007-04-03'),
        (10019,'1973-01-23','Lillian','Haddadi','M','2016-04-30'),
        (10020,'1972-12-24','Mayuko','Warwick','M','2011-01-26')

    )SQL").count();

    std::cout << employees << " have been added." << std::endl;

    std::cout << "The three oldest employees are: " << std::endl;

    auto &oldest = cnx.execute(R"SQL(

      SELECT first_name, last_name, DATE_PART('year', now()) - DATE_PART('year', birth_date)
        FROM employees
       ORDER BY birth_date
       LIMIT 3

    )SQL");

    for (auto &row: oldest) {
      std::cout << "- " << row.get<std::string>(0) << " " << row.get<std::string>(1)
        << ", " << row.get<double>(2) << " years old." << std::endl;
    }

    auto &employee = cnx.execute(R"SQL(

      SELECT first_name, last_name, DATE_PART('year', birth_date)
        FROM employees WHERE birth_date = $1::date

    )SQL", "1973-11-07");

    std::cout << employee.get<std::string>(0) << " "
      << employee.get<std::string>(1) << " is born in "
      << employee.get<double>(2) << std::endl;

    int deleted = cnx.execute(R"SQL(

      DELETE FROM employees
        WHERE DATE_PART('year', birth_date) = $1 AND gender = $2

    )SQL", 1973, 'M').count();

    std::cout << deleted << " employees records have been deleted." << std::endl;

    return 0;
  }
  catch (ConnectionException e) {
    std::cerr << "Oups... Cannot connect...";
  }
  catch (ExecutionException e) {
    std::cerr << "Oups... " << e.what();
  }

  return -1;
}
```

```
Table created.
20 have been added.
The three oldest employees are: 
- Sumant Peac, 44 years old.
- Mayuko Warwick, 44 years old.
- Lillian Haddadi, 43 years old.
Mary Sluis is born in 1973
2 employees records have been deleted.
```

## Compatibility

* `Linux x86_64` GCC 4.8.x
* `Mac x86_64` XCode 7.x
* `Windows x86_64` Visual Studio 2015 (work in progress)

## License
[MIT][license-mit]

[license-mit-img]: http://img.shields.io/badge/license-MIT-blue.svg?style=flat-square
[license-mit]: ./LICENSE.md