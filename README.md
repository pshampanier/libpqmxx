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


# See also
1. https://github.com/purpleKarrot/async-db
2. https://www.postgresql.org/docs/current/static/libpq-example.html
3. https://github.com/caanvaga/pgx
4. https://hackage.haskell.org/package/Takusen-0.7/docs/src/Database-PostgreSQL-PGFunctions.html
5. http://libpqtypes.esilo.com/browse_source.html?file=numerics.c

# Build

```
cmake -DPostgreSQL_ROOT=../dist/lib-postgresql-9.5.3/ ../
```

# Notes

```
SELECT  700::oid::regtype -- real
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