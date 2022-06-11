# t5-gsc-utils
T5 version of [t6-gsc-utils](https://github.com/fedddddd/t6-gsc-utils).  
If you wish for any feature to be added please create an [issue](https://github.com/fedddddd/t5-gsc-utils/issues/new).

# Installation
* Download the latest version from the [releases](https://github.com/fedddddd/t5-gsc-utils/releases)
* Copy it to `Plutonium/storage/t5/plugins`

# Features

* Easily add almost any C++ function into GSC:

  C++
  ```c++
  {
      gsc::function::add("myfunc", [](int a, const std::string& b)
      {
          printf("%i %s\n", a, b.data());
      });
  }
  ```
  
  GSC
  ```gsc
  init()
  {
      myFunc(1, "hello world");
  }
  ```
  
* Non-primitive GSC types such as **arrays** and **structs** (soon) are also supported and can be implicitly converted to STL containers:
  
  C++
  ```c++
  {
      gsc::function::add("sort", [](std::vector<int> vec)
      {
          std::sort(vec.begin(), vec.end());
          return vec;
      });
  }
  ```
  
  GSC
  ```gsc
  init()
  {
      arr = [];
      arr[arr.size] = 10;
      arr[arr.size] = 5;
      arr[arr.size] = 3;
      arr[arr.size] = 2;
      arr[arr.size] = 9;

      for (i = 0; i < arr.size; i++)
      {
          print(arr[i]);
      }

      print("");

      arr = sort(arr);
    
      for (i = 0; i < arr.size; i++)
      {
        print(arr[i]);
      }
  }
  ```
  
  More examples can be found [here](https://github.com/fedddddd/t5-gsc-utils/tree/main/src/component)
  
# Functions & Methods

A list of all the functions and methods that are added by this plugin.

## IO
  * `fileExists(path)`: Returns true if the file exists.
  * `writeFile(path, data[, append])`: Creates a file if it doesn't exist and writes/appends text to it.
  * `appendFile(path, data)`: Creates a file if it doesn't exist and appends text to it.
  * `readFile(path)`: Reads a file.
  * `fileSize(path)`: Returns file size in bytes.
  * `createDirectory(path)`: Creates a directory.
  * `directoryExists(path)`: Returns true if the directory exists.
  * `directoryIsEmpty(path)`: Returns true if the directory is empty.
  * `listFiles(path)`: Returns the list of files in the directory as an array.
  * `copyFolder(source, target)`, `copyDirectory(source, target)`: Copies a folder.
  * `removeDirectory(path[, recursive])`: Removes a directory and optionally all its contents.
## Command
  * `executeCommand(command)`: Executes a console command.

    ```gsc
    fast_restart()
    {
        executeCommand("fast_restart");
    }
    ```
  * `addCommand(name, callback)`: Adds a console command.

    ```gsc
    init()
    {
        addCommand("test_cmd", ::test_cmd);
    }
  
    test_cmd(args)
    {
        assert(args[0] == "test_cmd");
        print("Hello world", args.size);
    }
    ```
## String
  * `va(fmt, ...)`, `formatString(fmt, ...)`, `sprintf(fmt, ...)`: Formats a string:

    ```gsc
    init()
    {
        print(va("hello %i %f %s", 1, 1.2, "world"));
    }
    ```
  * `printf(fmt, ...)`: Formats and prints a string:

    ```gsc
    init()
    {
        printf("hello %i %f %s", 1, 1.2, "world");
    }
    ```
## Misc
  * `array(...)`: Creates an array from a list of arguments:

    ```gsc
    init()
    {
        arr = array(1, 2, 3, 4);
    }
    ```
  * `type(value)`, `typeof(value)`:

    ```gsc
    init()
    {
        assert(type("hello") == "string");
        assert(type(array()) == "array");
    }
    ```
## SOON
  * ...
