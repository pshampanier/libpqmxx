# <img src="help/images/libpqmxx-logo.png" height="60"/>

> A simple PostgreSQL client library written in Modern C++.

# Build

```
cmake -DPostgreSQL_ROOT=/dist/lib-postgresql-9.5.3/ ../
```

# Contributing

To run the test suite you will need to create a PostgreSQL user and a database named `ci-test`.

```bash
createuser ci-test
createdb ci-test --owner=ci-test --encoding=UTF8 
```

## OSX

```
  brew install postgresql
```
