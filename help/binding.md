# Bindings

   OID           | OID NUM  | SQL Type                    | C++ Param | C++ Result 
  ---------------+----------+-----------------------------+-----------+-----------
  BOOLOID        |       16 | boolean                     | bool      | bool
  BYTEAOID       |       17 | bytea        
  CHAROID        |       18 | "char"
  NAMEOID        |       19 | name
  INT8OID        |       20 | bigint                      | int64_t   | int64_t
  INT2OID        |       21 | smallint
  INT2VECTOROID  |       22 | int2vector
  INT4OID        |       23 | integer                     | int32_t   | int32_t
  REGPROCOID     |       24 | regproc
  TEXTOID        |       25 | text
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
  FLOAT4OID      |      700 | real
  FLOAT8OID      |      701 | double precision
  ABSTIMEOID     |      702 | abstime
  RELTIMEOID     |      703 | reltime
  TINTERVALOID   |      704 | tinterval
  UNKNOWNOID     |      705 | unknown
  CIRCLEOID      |      718 | circle
  CASHOID        |      790 | money
  INETOID        |      869 | inet
  CIDROID        |      650 | cidr
  BPCHAROID      |     1042 | character                 | const char * | std::string
  VARCHAROID     |     1043 | character varying         | const char * | std::string
  DATEOID        |     1082 | date
  TIMEOID        |     1083 | time without time zone
  TIMESTAMPOID   |     1114 | timestamp without time zone
  TIMESTAMPTZOID |     1184 | timestamp with time zone
  INTERVALOID    |     1186 | interval
  TIMETZOID      |     1266 | time with time zone
  ZPBITOID       |     1560 | bit
  VARBITOID      |     1562 | bit varying
  NUMERICOID     |     1700 | numeric
  
## TODO

### Warning
```
template1=# SELECT 'H' INTO tmp;
WARNING:  column "?column?" has type "unknown"
DETAIL:  Proceeding with relation creation anyway.
```

### Unknown type

```
template1=# SELECT 'H' AS col INTO tmp;
WARNING:  column "col" has type "unknown"
DETAIL:  Proceeding with relation creation anyway.
SELECT 1
template1=# \d tmp
      Table "public.tmp"
 Column |  Type   | Modifiers 
--------+---------+-----------
 col    | unknown | 
```