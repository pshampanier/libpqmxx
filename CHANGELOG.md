# Release 2.0

1. Support of asynchronous mode.

2. Notice Processing. You can now register a handler to process notices.

    ```c++
    cnx.notice([](const char *message) {
        std::cout << message << std::endl;    
    });
    ```
    
    By default, notices are displayed on stderr, to disable notices register nullptr as the notice handler:
    
    ```c++
    cnx.notice(nullptr);
    
    ```

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