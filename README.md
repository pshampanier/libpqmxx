# <img src="help/assets/libpqmxx-logo.png" height="60"/>


> A simple PostgreSQL client library written in Modern C++.

[![License][license-mit-img]][license-mit]

libpq**m**xx is un official C++ client API for ​PostgreSQL.

The aim of this library is to provide an API very simple to use without any compromise on performances. Modern C++ features such as variadic templates are used to make the programing interface very easy to use and the code slick to read.

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
    
* **`nullptr` can be used to set a null value**.

    ```cpp
    cnx.execute("INSERT INTO titles VALUES ($1, $2, $3::date, $4)",
                10020, "Technique Leader", "1988-02-10", nullptr);
    ```


The online API documentation is available on [gitbook](https://pshampanier.gitbooks.io/libpqmxx/content/).  
If you are looking for the official C++ client library fro PostgreSQL, please visit [pqxx.org](http://pqxx.org).

## Compatibility

* `Linux x86_64` GCC 5.x
* `Mac x86_64` XCode 7.x
* `Windows x86_64` Visual Studio 2015 (work in progress)

## License
[MIT][license-mit]

[license-mit-img]: http://img.shields.io/badge/license-MIT-blue.svg?style=flat-square
[license-mit]: ./LICENSE.md