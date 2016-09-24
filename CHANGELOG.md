# 1.1.1

1. Fixed compilation with MINGW (#6, thank you @e-fominov).

# Release 1.1

1. Adding support of arrays.

  ```c++
    auto result = cnx.execute("SELECT ARRAY[4,null,6]");
    auto values = result.asArray<int32_t>(0);
  ```
  
2. `Row::get<T>` is deprecated and replaced by `Row::as<T>`.

3. Adding library runtime settings. One setting is currenly supported to handle empty stings as null values:

    ```c++
    struct Settings {
      /**
       * If true (the default), empty strings passed as parameter to execute() are
       * considered null values.
       * 
       * ```
       * // the following code will set the `first_name` to null.
       * std::string name("");
       * cnx.execute("UPDATE employee SET first_name=$1 WHERE emp_no=$2", name, 10020);
       * ```
       **/
      bool emptyStringAsNull = true;
    };

    ```