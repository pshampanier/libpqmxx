# Bindings

   OID           | OID NUM  | SQL Type                    | C++ Param             | C++ Result 
  ---------------|----------|-----------------------------|-----------------------|-----------
  BOOLOID        |       16 | boolean                     | bool                  | bool
  BYTEAOID       |       17 | bytea                       | std::vector\<uint_8\> | std::vector\<uint_8\>
  CHAROID        |       18 | "char"                      | char                  | char
  NAMEOID        |       19 | name                        | const char *          | std::string
  INT8OID        |       20 | bigint                      | int64_t               | int64_t
  INT2OID        |       21 | smallint                    | int16_t               | int16_t  
  INT2VECTOROID  |       22 | int2vector
  INT4OID        |       23 | integer                     | int32_t               | int32_t
  REGPROCOID     |       24 | regproc
  TEXTOID        |       25 | text                        | const char *          | std::string
  OIDOID         |       26 | oid
  TIDOID         |       27 | tid
  XIDOID         |       28 | xid
  CIDOID         |       29 | cid
  OIDVECTOROID   |       30 | oidvector
  JSON8OID       |      114 | json
  POINTOID       |      600 | point
  LSEGOID        |      601 | lseg
  PATHOID        |      602 | path
  BOXOID         |      603 | box
  POLYGONOID     |      604 | polygon
  LINEOID        |      628 | line
  FLOAT4OID      |      700 | real                        | float                       | float
  FLOAT8OID      |      701 | double precision            | double                      | double
  ABSTIMEOID     |      702 | abstime
  RELTIMEOID     |      703 | reltime
  TINTERVALOID   |      704 | tinterval
  UNKNOWNOID     |      705 | unknown
  CIRCLEOID      |      718 | circle
  CASHOID        |      790 | money
  INETOID        |      869 | inet
  CIDROID        |      650 | cidr
  BPCHAROID      |     1042 | character                   | const char *                | std::string
  VARCHAROID     |     1043 | character varying           | const char *                | std::string
  DATEOID        |     1082 | date                        | db::postgres::date_t        | db::postgres::date_t
  TIMEOID        |     1083 | time without time zone      | db::postgres::time_t        | db::postgres::time_t
  TIMESTAMPOID   |     1114 | timestamp without time zone | db::postgres::timestamp_t   | db::postgres::timestamp_t
  TIMESTAMPTZOID |     1184 | timestamp with time zone    | db::postgres::timestamptz_t | db::postgres::timestamptz_t
  INTERVALOID    |     1186 | interval                    | db::postgres::interval_t    | db::postgres::interval_t
  TIMETZOID      |     1266 | time with time zone         | db::postgres::timetz_t      | db::postgres::timetz_t
  ZPBITOID       |     1560 | bit
  VARBITOID      |     1562 | bit varying
  NUMERICOID     |     1700 | numeric

## Other bindings

SQL Type      | C++ Param | C++ Result 
--------------|-----------|-----------------
smallserial   | n/a       | int16_t
serial        | n/a       | int32_t
bigserial     | n/a       | int64_t


