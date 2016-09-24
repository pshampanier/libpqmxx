# Contributing

Use [cmake](https://cmake.org).

If cmake cannot find postgresql library, you can define `PostgreSQL_ROOT`: 

```
cmake -DPostgreSQL_ROOT=/dist/lib-postgresql-9.5.3/ -DGTEST_ROOT=/dist/lib-gtest
```

To run the test suite you will need to create a PostgreSQL user and a database named `ci-test`.

```bash
createuser ci-test
createdb ci-test --owner=ci-test --encoding=UTF8 
```

You can set the location of the PostgreSQL server by using 
[Environment Variables](https://www.postgresql.org/docs/current/static/libpq-envars.html).

### Dependancies

* MacOS: `brew install postgresql`.
* Linux: `apt-get install postgresql-server-dev-9.5`.