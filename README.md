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

# Build
```
cmake -G Xcode \
  -DPostgreSQL_INCLUDE_DIRS=../dist/lib-postgresql-9.5.3/include/ \
  -DPostgreSQL_LIBRARIES=../dist/lib-postgresql-9.5.3/lib/libpq.a \
  -DOPENSSL_LIBRARIES="../dist/lib-openssl-1.0.2h/libs/Release/libssl.a;../dist/lib-openssl-1.0.2h/libs/Release/libcrypto.a" \
  ../
```