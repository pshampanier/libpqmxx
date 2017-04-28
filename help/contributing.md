# Contributing

Use [cmake](https://cmake.org).

If cmake cannot find libraries, you can help by defining `PostgreSQL_ROOT`, `GTEST_ROOT` 
or `BOOST_ROOT`: 

```
cmake -DPostgreSQL_ROOT=/libpq -DGTEST_ROOT=/lib-gtest -DBOOST_ROOT=/lib-boost
```

To run the test suite you will need to create a PostgreSQL user and a database named `ci-test`.

```bash
createuser ci-test
createdb ci-test --owner=ci-test --encoding=UTF8 
```

And modify the `pg_hba.conf` to trust this user on this database:

```
# TYPE  DATABASE		USER			ADDRESS		METHOD
host 	ci-test 		ci-test 		all			trust
``` 

### Dependancies

* MacOS: `brew install postgresql`.
* Linux: `apt-get install postgresql-server-dev-9.5`.