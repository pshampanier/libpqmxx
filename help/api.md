# namespace `postgres`



## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`class `[``Connection``](#classdb_1_1postgres_1_1_connection)    | A connection to a PostgreSQL database.
`class `[``ConnectionException``](#classdb_1_1postgres_1_1_connection_exception)    | Exception thrown on connection failure.
`class `[``ExecutionException``](#classdb_1_1postgres_1_1_execution_exception)    | Exception thrown on any runtime error except a connection failure.
`class `[``Result``](#classdb_1_1postgres_1_1_result)    | A result from an SQL command.
`class `[``Row``](#classdb_1_1postgres_1_1_row)    | A row in a [Result](#classdb_1_1postgres_1_1_result).
`struct `[``array_item``](#structdb_1_1postgres_1_1array__item)    | A values in an array.
`struct `[``date_t``](#structdb_1_1postgres_1_1date__t)    | A `date` value.
`struct `[``interval_t``](#structdb_1_1postgres_1_1interval__t)    | An `interval` value.
`struct `[``Settings``](#structdb_1_1postgres_1_1_settings)    | [Settings](#structdb_1_1postgres_1_1_settings) of a PostgreSQL connection.
`struct `[``time_t``](#structdb_1_1postgres_1_1time__t)    | A `time` value.
`struct `[``timestamp_t``](#structdb_1_1postgres_1_1timestamp__t)    | A `timestamp` value (without time zone).
`struct `[``timestamptz_t``](#structdb_1_1postgres_1_1timestamptz__t)    | A `timestamp with timezone` value.
`struct `[``timetz_t``](#structdb_1_1postgres_1_1timetz__t)    | A `time with timezone` value.

# class `Connection` {#classdb_1_1postgres_1_1_connection}

```
class Connection
  : public std::enable_shared_from_this< Connection >
```  

A connection to a PostgreSQL database.



## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public  Connection(`[`Settings`](#structdb_1_1postgres_1_1_settings)` settings)` | Constructor.
`public  ~Connection()` | Destructor.
`public `[`Connection`](#classdb_1_1postgres_1_1_connection)` & connect(const char * connInfo)` | Open a connection to the database.
`public `[`Connection`](#classdb_1_1postgres_1_1_connection)` & close() noexcept` | Close the database connection.
`public `[`Connection`](#classdb_1_1postgres_1_1_connection)` & cancel()` | Cancel queries in progress.
`public template<typename... Args>`  <br/>`inline `[`Result`](#classdb_1_1postgres_1_1_result)` & execute(const char * sql,Args... args)` | Execute one or more SQL commands.
`public `[`Connection`](#classdb_1_1postgres_1_1_connection)` & begin()` | Start a transaction.
`public `[`Connection`](#classdb_1_1postgres_1_1_connection)` & commit()` | Commit a transaction.
`public `[`Connection`](#classdb_1_1postgres_1_1_connection)` & rollback()` | Rollback a transaction.
`protected PGconn * pgconn_` | The native connection pointer.
`protected `[`Result`](#classdb_1_1postgres_1_1_result)` result_` | [Result](#classdb_1_1postgres_1_1_result) of the current query.
`protected std::string lastError() const noexcept` | Last error messages sent by the server.
`protected inline  operator PGconn *()` | Cast operator to the native connection pointer.

## Members

### `public  Connection(`[`Settings`](#structdb_1_1postgres_1_1_settings)` settings)` {#classdb_1_1postgres_1_1_connection_1ac55be960c65f00b17772cc5cb4fdbb62}

Constructor.

Copy and move constructor have been explicitly deleted to prevent the copy of the connection object.

### `public  ~Connection()` {#classdb_1_1postgres_1_1_connection_1ab0771ae2470a6ee9625a8adfd551f496}

Destructor.

The destuctor will implicitly close the connection to the server and associated results if necessary.

### `public `[`Connection`](#classdb_1_1postgres_1_1_connection)` & connect(const char * connInfo)` {#classdb_1_1postgres_1_1_connection_1aaf2fe1607169b7118735655bd695576b}

Open a connection to the database.

#### Parameters
* `connInfo` A postgresql connection string. Both Keyword/Value and URI are accepted. The passed `connInfo` can be empty or null to use all default parameters (see [Environment Variables](https://www.postgresql.org/docs/9.5/static/libpq-envars.html)).





```cpp
postgresql://[user[:password]@][netloc][:port][/dbname][?param1=value1&...]
```



**See also**: [https://www.postgresql.org/docs/9.5/static/libpq-connect.html#LIBPQ-CONNSTRING](https://www.postgresql.org/docs/9.5/static/libpq-connect.html#LIBPQ-CONNSTRING)


#### Returns
The connection itself.

### `public `[`Connection`](#classdb_1_1postgres_1_1_connection)` & close() noexcept` {#classdb_1_1postgres_1_1_connection_1a31910dd96d809b69c13d01a59941a335}

Close the database connection.

This method is automatically called by the destructor.

### `public `[`Connection`](#classdb_1_1postgres_1_1_connection)` & cancel()` {#classdb_1_1postgres_1_1_connection_1a0cf67cd39e3d63fa5ffc794fd7a02493}

Cancel queries in progress.

Calling this method will request the cancellation of the current query in progress. There is no guaranty the query will be cancelled or when it will be cancelled. So usually once you've called this method you can't use anymore the connection since the previous query might still be in progress.


#### Returns
The connection ifself.

### `public template<typename... Args>`  <br/>`inline `[`Result`](#classdb_1_1postgres_1_1_result)` & execute(const char * sql,Args... args)` {#classdb_1_1postgres_1_1_connection_1a3a0df5f68403ff370bef5026eec584c2}

Execute one or more SQL commands.

#### Parameters
* `sql` One or more SQL commands to be executed. 


* `args` Zero or more parameters of the SQL command. If parameters are used, they are referred to in the `sql` command as `$1`, `$2`, etc... The PostgreSQL datatype of a parameter is deducted from the C++ type of the parameter according to the following table:





SQL Type |C++ Param
--------- | ---------
boolean |bool
bytea |std::vector<uint_8>
"char" |char
bigint |int64_t
smallint |int16_t
integer |int32_t
real |float
double precision |double
character varying |const char *, std::string
date |[db::postgres::date_t](#structdb_1_1postgres_1_1date__t)
time without time zone |[db::postgres::time_t](#structdb_1_1postgres_1_1time__t)
timestamp without time zone |[db::postgres::timestamp_t](#structdb_1_1postgres_1_1timestamp__t)
timestamp with time zone |[db::postgres::timestamptz_t](#structdb_1_1postgres_1_1timestamptz__t)
interval |[db::postgres::interval_t](#structdb_1_1postgres_1_1interval__t)
time with time zone |[db::postgres::timetz_t](#structdb_1_1postgres_1_1timetz__t)


```cpp
// Execute a query
auto results = cnx.execute("SELECT MAX(emp_no) FROM employees");

// Execute a query with parameters.
cnx.execute("UPDATE titles SET to_date=$1::date WHERE emp_no=$2", "1988-02-10", 10020);

// Execute multiple statements in one call.
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

)SQL");
```
 When the parameter data type might be ambigious for the server, you can use the PostgreSQL casting syntax. In the example below both parameters are send as `character varying` but the server will perform the cast to `date` and `integer`. 
```cpp
cnx.execute("UPDATE titles SET to_date=$1::date WHERE emp_no::integer=$2", "1988-02-10", "10020");
```


The `null` value can be send as a parameter using the `nullptr` keyword. 
```cpp
cnx.execute("INSERT INTO titles VALUES ($1, $2, $3::date, $4)",
            10020, "Technique Leader", "1988-02-10", nullptr);
```


> Parameters are only supported for a single SQL command. If [execute()](#classdb_1_1postgres_1_1_connection_1a3a0df5f68403ff370bef5026eec584c2) is called with more than one sql command and any of those commands are using parameters, [execute()](#classdb_1_1postgres_1_1_connection_1a3a0df5f68403ff370bef5026eec584c2) will fail.


#### Returns
The results of the SQL commands.


You can iterate over the returned result using the [Result::iterator](#classdb_1_1postgres_1_1_result_1_1iterator). When a query in a the `sql` command is a SELECT all the rows of the result must be fetched using the iterator before the connection can be reused for another command. For other commands such as an UPDATE or a CREATE TABLE, using the iterator is not required and the connection can be reused for the next command right away.

### `public `[`Connection`](#classdb_1_1postgres_1_1_connection)` & begin()` {#classdb_1_1postgres_1_1_connection_1a086d2a64021d88fa15889439143f66ac}

Start a transaction.

[begin()](#classdb_1_1postgres_1_1_connection_1a086d2a64021d88fa15889439143f66ac), [commit()](#classdb_1_1postgres_1_1_connection_1aeff7ff857a2224dcffeeadcd96a09f14) and [rollback()](#classdb_1_1postgres_1_1_connection_1a308e05aa25244cc560d8def07d349844) are helper methods to deal with transactions. The main benefit of using those methods rather than executing the SQL commands is on nested transactions. If a code start a transaction and call another code also starting a transaction, thoses methods will create only one transaction started at the first all to [begin()](#classdb_1_1postgres_1_1_connection_1a086d2a64021d88fa15889439143f66ac) and commited at the last call to [commit()](#classdb_1_1postgres_1_1_connection_1aeff7ff857a2224dcffeeadcd96a09f14).


#### Returns
The connection itself.

### `public `[`Connection`](#classdb_1_1postgres_1_1_connection)` & commit()` {#classdb_1_1postgres_1_1_connection_1aeff7ff857a2224dcffeeadcd96a09f14}

Commit a transaction.

#### Returns
The connection itself.

### `public `[`Connection`](#classdb_1_1postgres_1_1_connection)` & rollback()` {#classdb_1_1postgres_1_1_connection_1a308e05aa25244cc560d8def07d349844}

Rollback a transaction.

#### Returns
The connection itself.

### `protected PGconn * pgconn_` {#classdb_1_1postgres_1_1_connection_1aebe710e91ed7a7978d585ddeaffaab2b}

The native connection pointer.



### `protected `[`Result`](#classdb_1_1postgres_1_1_result)` result_` {#classdb_1_1postgres_1_1_connection_1aa29316537e596fd8f4dcc51950da3612}

[Result](#classdb_1_1postgres_1_1_result) of the current query.



### `protected std::string lastError() const noexcept` {#classdb_1_1postgres_1_1_connection_1a860cf6e150e5b3e230fb2a6cfe2c2b93}

Last error messages sent by the server.

#### Returns
The error message most recently generated by an operation on the connection.

### `protected inline  operator PGconn *()` {#classdb_1_1postgres_1_1_connection_1ae8ae6e3848a5d74a0460fd4d90224092}

Cast operator to the native connection pointer.




# class `ConnectionException` {#classdb_1_1postgres_1_1_connection_exception}

```
class ConnectionException
  : public runtime_error
```  

Exception thrown on connection failure.



## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  ConnectionException(const std::string & what)` | Constructor.

## Members

### `public inline  ConnectionException(const std::string & what)` {#classdb_1_1postgres_1_1_connection_exception_1a097ce2cfb8aa8b04452f912652ad484b}

Constructor.

#### Parameters
* `what` - The error message returned by the server.


# class `ExecutionException` {#classdb_1_1postgres_1_1_execution_exception}

```
class ExecutionException
  : public runtime_error
```  

Exception thrown on any runtime error except a connection failure.



## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  ExecutionException(const std::string & what)` | Constructor.

## Members

### `public inline  ExecutionException(const std::string & what)` {#classdb_1_1postgres_1_1_execution_exception_1ac268e6bb3babf3cc98aa7e4f01e4f8e3}

Constructor.

#### Parameters
* `what` - The error message returned by the server.


# class `Result` {#classdb_1_1postgres_1_1_result}

```
class Result
  : public db::postgres::Row
```  

A result from an SQL command.

SQL commands always return a result. For a SELECT command you can iterate through the result using the [Result::iterator](#classdb_1_1postgres_1_1_result_1_1iterator). Otherwise you can use the [count()](#classdb_1_1postgres_1_1_result_1af66b6b7b685ee6d1d0fe2f517b53557e) method to get the number of rows affected by the SQL command.

When executing more than one SQL command, the iterator can also be used to access the result of each command.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`class `[``iterator``](#classdb_1_1postgres_1_1_result_1_1iterator)        | Support of the range-based for loops.
`public uint64_t count() const noexcept` | Number of rows affected by the SQL command.
`public `[`iterator`](#classdb_1_1postgres_1_1_result_1_1iterator)` begin()` | First row of the result.
`public `[`iterator`](#classdb_1_1postgres_1_1_result_1_1iterator)` end()` | Last row of the result.

## Members

### `class `[``iterator``](#classdb_1_1postgres_1_1_result_1_1iterator) {#classdb_1_1postgres_1_1_result_1_1iterator}

Support of the range-based for loops.

```cpp
auto &result = cnx.exectute("SELECT ...");
for (auto &row: result) {
  ...
}
```
### `public uint64_t count() const noexcept` {#classdb_1_1postgres_1_1_result_1af66b6b7b685ee6d1d0fe2f517b53557e}

Number of rows affected by the SQL command.

This function can only be used following the execution of a SELECT, CREATE TABLE AS, INSERT, UPDATE, DELETE, MOVE, FETCH, or COPY statement, or an EXECUTE of a prepared query that contains an INSERT, UPDATE, or DELETE statement. If the command that generated the PGresult was anything else, [count()](#classdb_1_1postgres_1_1_result_1af66b6b7b685ee6d1d0fe2f517b53557e) returns 0.


#### Returns
The number of rows affected by the SQL statement.

### `public `[`iterator`](#classdb_1_1postgres_1_1_result_1_1iterator)` begin()` {#classdb_1_1postgres_1_1_result_1abad750449a1aa1814b1d080f1626a19c}

First row of the result.

#### Returns
An iterator pointing to the first row of the result.

### `public `[`iterator`](#classdb_1_1postgres_1_1_result_1_1iterator)` end()` {#classdb_1_1postgres_1_1_result_1a13b8ac4b95d89cca7213271dc98909d8}

Last row of the result.

#### Returns
An iterator pointing to the past-the-end row of the result.


# class `iterator` {#classdb_1_1postgres_1_1_result_1_1iterator}


Support of the range-based for loops.

```cpp
auto &result = cnx.exectute("SELECT ...");
for (auto &row: result) {
  ...
}
```

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  iterator(`[`Row`](#classdb_1_1postgres_1_1_row)` * ptr)` | Constructor.
`public `[`iterator`](#classdb_1_1postgres_1_1_result_1_1iterator)` operator++()` | Next row in the resultset.
`public inline bool operator!=(const `[`iterator`](#classdb_1_1postgres_1_1_result_1_1iterator)` & other)` | 
`public inline `[`Row`](#classdb_1_1postgres_1_1_row)` & operator*()` | 

## Members

### `public inline  iterator(`[`Row`](#classdb_1_1postgres_1_1_row)` * ptr)` {#classdb_1_1postgres_1_1_result_1_1iterator_1a8edbc51a3fcbc5338ec3fefd5ae7888c}

Constructor.



### `public `[`iterator`](#classdb_1_1postgres_1_1_result_1_1iterator)` operator++()` {#classdb_1_1postgres_1_1_result_1_1iterator_1a27714a8124ae0a4b04ecc863d5bdc5cd}

Next row in the resultset.



### `public inline bool operator!=(const `[`iterator`](#classdb_1_1postgres_1_1_result_1_1iterator)` & other)` {#classdb_1_1postgres_1_1_result_1_1iterator_1a1e1dcab41e0d7b58f56e4cf1be2dcba9}





### `public inline `[`Row`](#classdb_1_1postgres_1_1_row)` & operator*()` {#classdb_1_1postgres_1_1_result_1_1iterator_1a9100a2370c979582f3ad8de0a3539bac}






# class `Row` {#classdb_1_1postgres_1_1_row}


A row in a [Result](#classdb_1_1postgres_1_1_result).

Rows can be accessed using the [Result::iterator](#classdb_1_1postgres_1_1_result_1_1iterator) except the first one that can be directly access through the result.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public bool isNull(int column) const` | Test a column for a null value.
`public template<typename T>`  <br/>`T as(int column) const` | Get a column value.
`public template<typename T>`  <br/>`std::vector< `[`array_item`](#structdb_1_1postgres_1_1array__item)`< T > > asArray(int column) const` | Get a column values for arrays.
`public const char * columnName(int column) const` | Get a column name.
`public int num() const noexcept` | Get the row number.
`public template<typename T>`  <br/>`inline T get(int column) const` | Get a column value.

## Members

### `public bool isNull(int column) const` {#classdb_1_1postgres_1_1_row_1a38e8b7b279e1a05b6a7abdc995238e3e}

Test a column for a null value.

#### Parameters
* `column` Column number. Column numbers start at 0. 





#### Returns
true if the column value is a null value.

### `public template<typename T>`  <br/>`T as(int column) const` {#classdb_1_1postgres_1_1_row_1abe01bd7b402aabbadf98c9e6805e22ae}

Get a column value.

```cpp
int32_t emp_no = row.as<int32_t>(0);
```


The `typename` used to call the function must by compatible with the SQL type. The following table defines compatible types:


SQL Type |Typename |null value
--------- | --------- | ---------
boolean |bool |false
bytea |std::vector<uint_8> |*empty vector*
"char" |char |'\0'
name |std::string |*empty string*
bigint |int64_t |0
smallint |int16_t |0
integer |int32_t |0
text |std::string |*empty string*
real |float |0.f
double precision |double |0.
character |std::string |*empty string*
character varying |std::string |*empty string*
date |[db::postgres::date_t](#structdb_1_1postgres_1_1date__t)|{ 0 }
time without time zone |[db::postgres::time_t](#structdb_1_1postgres_1_1time__t)|{ 0 }
timestamp without time zone |[db::postgres::timestamp_t](#structdb_1_1postgres_1_1timestamp__t)|{ 0 }
timestamp with time zone |[db::postgres::timestamptz_t](#structdb_1_1postgres_1_1timestamptz__t)|{ 0 }
interval |[db::postgres::interval_t](#structdb_1_1postgres_1_1interval__t)|{ 0, 0 }
time with time zone |[db::postgres::timetz_t](#structdb_1_1postgres_1_1timetz__t)|{ 0, 0, 0 }
smallserial |int16_t |0
serial |int32_t |0
bigserial |int64_t |0
If the column value is null, the null value defined in the table above will be returned. To insure the column value is really null the method [isNull()](#classdb_1_1postgres_1_1_row_1a38e8b7b279e1a05b6a7abdc995238e3e) should be used.


#### Parameters
* `column` Column number. Column numbers start at 0. 





#### Returns
The value of the column.

> Calling this method with incompatible types is treated as programming errors, not user or run-time errors. Those errors will be captured by an assert in debug mode and the behavior in non debug modes is undertermined.

### `public template<typename T>`  <br/>`std::vector< `[`array_item`](#structdb_1_1postgres_1_1array__item)`< T > > asArray(int column) const` {#classdb_1_1postgres_1_1_row_1abc7c15eb90efcd69ea605524918da381}

Get a column values for arrays.

```cpp
array_int32_t quarters = row.asArray<int32_t>(0);
```


Usage is the similar to using `T as(int column)` and binding between SQL names and C++ types are the same. Only bytea is not supported.

Only array of one dimention are supported.


#### Parameters
* `column` Column number. Column numbers start at 0. 





#### Returns
The value of the column. The value is actually a vector of array_item<T>. Each item has two properties (`value` and `isNull`).

### `public const char * columnName(int column) const` {#classdb_1_1postgres_1_1_row_1a36da64e5fead10d999dd41e3e1cd6adf}

Get a column name.

#### Parameters
* `column` Column number. Column numbers start at 0. 





#### Returns
The column name associated with the given column number.


```cpp
cnx.execute("SELECT emp_no, firstname from employees").columnName(1) // → "firstname"
```

### `public int num() const noexcept` {#classdb_1_1postgres_1_1_row_1a450ab7a4e2945826f8c4f35cb8af9cf4}

Get the row number.

#### Returns
For each row returned by a query, [num()](#classdb_1_1postgres_1_1_row_1a450ab7a4e2945826f8c4f35cb8af9cf4) returns a number indicating the order of the row in the result. The first row selected has a [num()](#classdb_1_1postgres_1_1_row_1a450ab7a4e2945826f8c4f35cb8af9cf4) of 1, the second has 2, and so on.

### `public template<typename T>`  <br/>`inline T get(int column) const` {#classdb_1_1postgres_1_1_row_1afc50bb75e1ed70b0d718c4c600268e47}

Get a column value.

> Deprecated: use Row::as(int column) for replacement.


# struct `array_item` {#structdb_1_1postgres_1_1array__item}


A values in an array.



## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public T value` | The value for non null values.
`public bool isNull` | `true` if the value is null.
`public  array_item() = default` | 
`public inline  array_item(T v)` | Constructor of a non null value.
`public inline  array_item(std::nullptr_t)` | Constructor of a null value.
`public inline  operator const T &() const` | 
`public inline bool operator==(const `[`array_item`](#structdb_1_1postgres_1_1array__item)`< T > & other) const` | 

## Members

### `public T value` {#structdb_1_1postgres_1_1array__item_1a12b55fab4d1ed6bd7f5cf22c20d5c3fc}

The value for non null values.



### `public bool isNull` {#structdb_1_1postgres_1_1array__item_1a981900c5341377d5cb5d32ffe6bafd25}

`true` if the value is null.



### `public  array_item() = default` {#structdb_1_1postgres_1_1array__item_1a1147b2c5aa0503b228743b575133866f}





### `public inline  array_item(T v)` {#structdb_1_1postgres_1_1array__item_1aedae122292fef60bb1b9854675b19b3d}

Constructor of a non null value.

#### Parameters
* `v` The value.

### `public inline  array_item(std::nullptr_t)` {#structdb_1_1postgres_1_1array__item_1a523d134253b40d890ca94b30e858944b}

Constructor of a null value.



### `public inline  operator const T &() const` {#structdb_1_1postgres_1_1array__item_1a69514ebbf7a1e4f50748cca26ac312a4}





### `public inline bool operator==(const `[`array_item`](#structdb_1_1postgres_1_1array__item)`< T > & other) const` {#structdb_1_1postgres_1_1array__item_1a409565908e45d02e600d6e47a6d49aa9}






# struct `date_t` {#structdb_1_1postgres_1_1date__t}


A `date` value.

This struct can be used set a date parameter when calling execute, or to get a date value from a [Row](#classdb_1_1postgres_1_1_row). An alternative for date parameters is to use a date literal with an explict cast of the parameter in the sql command.


```cpp
execute("SELECT $1::date", "2014-11-01");
```

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public int32_t epoch_date` | Number of seconds sine Unix epoch time.
`public inline  operator int32_t() const` | Cast to `int32_t`.

## Members

### `public int32_t epoch_date` {#structdb_1_1postgres_1_1date__t_1a4850957397f150ba6f4e128595f6375f}

Number of seconds sine Unix epoch time.



### `public inline  operator int32_t() const` {#structdb_1_1postgres_1_1date__t_1a794f59a7d479feac29d4e9f60dde8738}

Cast to `int32_t`.




# struct `interval_t` {#structdb_1_1postgres_1_1interval__t}


An `interval` value.



## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public int64_t time` | Number of microseconds on the day since 00:00:00.
`public int32_t days` | Number of days.
`public int32_t months` | Number of months.

## Members

### `public int64_t time` {#structdb_1_1postgres_1_1interval__t_1aca3c9fae42007d77adc4f086c9300c48}

Number of microseconds on the day since 00:00:00.



### `public int32_t days` {#structdb_1_1postgres_1_1interval__t_1afaede2f5d53642637f3cac07cc2e73ec}

Number of days.



### `public int32_t months` {#structdb_1_1postgres_1_1interval__t_1a032c05d15f12d1eca703fe005633af36}

Number of months.




# struct `Settings` {#structdb_1_1postgres_1_1_settings}


[Settings](#structdb_1_1postgres_1_1_settings) of a PostgreSQL connection.

Those settings are internal setting of the libpqmxx library. Other standard PostgreSQL seetings should be set when calling connect().

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public bool emptyStringAsNull` | If true, empty strings passed as parameters to exectute() are considered as null values.

## Members

### `public bool emptyStringAsNull` {#structdb_1_1postgres_1_1_settings_1a5c20471464317fd999d1d39d9560bfed}

If true, empty strings passed as parameters to exectute() are considered as null values.




# struct `time_t` {#structdb_1_1postgres_1_1time__t}


A `time` value.



## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public int64_t time` | Number of microseconds since 00:00:00.
`public inline  operator int64_t() const` | Cast to int64_t.

## Members

### `public int64_t time` {#structdb_1_1postgres_1_1time__t_1a1d79e495efb93363589ccb4861c8c064}

Number of microseconds since 00:00:00.



### `public inline  operator int64_t() const` {#structdb_1_1postgres_1_1time__t_1a90e8f8db0e1cb44d3e00bca188c539c9}

Cast to int64_t.




# struct `timestamp_t` {#structdb_1_1postgres_1_1timestamp__t}


A `timestamp` value (without time zone).

This struct can be used set a timestamp parameter when calling execute, or to get a timestamp value from a [Row](#classdb_1_1postgres_1_1_row). An alternative for timestamp parameters is to use a timestamp literal with an explict cast of the parameter in the sql command.


```cpp
execute("SELECT $1::timestamp", "2014-11-01T05:19:00");
```

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public int64_t epoch_time` | Number of microsecondes since Unix epoch time.
`public inline  operator int64_t() const` | Cast to int64_t.

## Members

### `public int64_t epoch_time` {#structdb_1_1postgres_1_1timestamp__t_1ac8e9ef9330324c4134fcb763b3322436}

Number of microsecondes since Unix epoch time.



### `public inline  operator int64_t() const` {#structdb_1_1postgres_1_1timestamp__t_1a378561481926a4781ae1bfda1f6b98cd}

Cast to int64_t.




# struct `timestamptz_t` {#structdb_1_1postgres_1_1timestamptz__t}


A `timestamp with timezone` value.

This struct can be used set a timestamptz parameter when calling execute, or to get a timestamptz value from a [Row](#classdb_1_1postgres_1_1_row). An alternative for timestamptz parameters is to use a timestamp literal with an explict cast of the parameter in the sql command.


```cpp
execute("SELECT $1::timestamptz", "2014-11-01T05:19:00-500");
```

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public int64_t epoch_time` | Number of microsecondes since Unix epoch time.
`public inline  operator int64_t() const` | Cast to int64_t.

## Members

### `public int64_t epoch_time` {#structdb_1_1postgres_1_1timestamptz__t_1aa2c055460381e818d3df1776a9a4ea3e}

Number of microsecondes since Unix epoch time.



### `public inline  operator int64_t() const` {#structdb_1_1postgres_1_1timestamptz__t_1aaf782e90ce2ba8b3acbf63f36c15ce9b}

Cast to int64_t.




# struct `timetz_t` {#structdb_1_1postgres_1_1timetz__t}


A `time with timezone` value.



## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public int64_t time` | Number of microseconds since 00:00:00.
`public int32_t offset` | Offset from GMT in seconds.

## Members

### `public int64_t time` {#structdb_1_1postgres_1_1timetz__t_1abe67faa78687684a6d008e052adee346}

Number of microseconds since 00:00:00.



### `public int32_t offset` {#structdb_1_1postgres_1_1timetz__t_1aa34f0a81487d034137ad0170aacfe5a4}

Offset from GMT in seconds.




