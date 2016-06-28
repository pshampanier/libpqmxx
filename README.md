# db-postgres-cpp

> A PostgreSQL client written in Modern C++ supporting synchonous and asynchronous processing.

```cpp
auto cnx = std::make_shared(db::postgres::Connection());

cnx.onError([cnx](std::exception_ptr reason) {
  // do something with the error
});

cnx.connect("postgres://", [cnx]() {
    cnx.execute("CREATE TABLE mytable(c1 VARCHAR 10)", [cnx](int) {
      cnx.execute("INSERT INTO mytable (c1) VALUES (:1)", "hello", [cnx](int count) {
      
      });
    });
});
```
```cpp
cnx.connect("postgres://").done([cnx]() {
  cnx.execute("CREATE TABLE mytable(c1 VARCHAR 10)").done([cnx]() {
  
  });
}).error([] {

});
```

# Build

```
cmake -DPostgreSQL_ROOT=/dist/lib-postgresql-9.5.3/ ../
```

# Contributing

Create a user and a database `ci-test`.

```bash
createuser ci-test
createdb ci-test --owner=ci-test --encoding=UTF8 
```

## OSX

```
  brew install postgresql
```
