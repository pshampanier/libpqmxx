# Release 1.1

1. Adding support of arrays.

  ```c++
    auto result = cnx.execute("SELECT ARRAY[4,null,6]");
    std::vector<int32_t> values = result.asArray<int32_t>(0);
    std::vector<bool> nulls = result.nullValues(0);    
  ```
  

2. `Row::get<T>` is deprecated and replaced by `Row::as<T>`.