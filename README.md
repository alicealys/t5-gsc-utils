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
  * `executeCommand(command)`, `command::execute(command)`: Executes a console command.

    ```gsc
    fast_restart()
    {
        executeCommand("fast_restart");
    }
    ```
  * `addCommand(name, callback)`, `command::add(name, callback)`: Adds a console command.

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
  * `addClientCommand(name, callback)`, `command::add_sv(name, callback)`: Adds a client command (can be executed by players through the console).

    ```gsc
    init()
    {
        command::add_sv("suicide", ::sv_cmd_suicide);
    }
    
    sv_cmd_suicide(args)
    {
        self suicide();
    }
    ```
  * `self tell(message)`: Prints a message to a player's chat.
  * `say(message)`: Prints a message to all players' chat.

    ```gsc
    init()
    {
        level thread on_player_connected();
    }
  
    on_player_connected()
    {
        while (true)
        {
            level waittill("connected", player);
            say(string::format("%s connected!", player.name));
            player tell(string::format("Hello %s!", player.name));
        }
    }
    ```
  * `sendServerCommand(client_num, reliable, text)`: Executes SV_GameSendServerCommand.
## String
  * `va(fmt, ...)`, `formatString(fmt, ...)`, `sprintf(fmt, ...)`  
    `string::va(fmt, ...)`, `string::format(fmt, ...)`: Formats a string:

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
  * `tolower(string)`, `string::to_lower(string)`: Converts the string to lowercase.
  * `toupper(string)`, `string::to_upper(string)`: Converts the string to uppercase.
  * `string::is_numeric(string)`: Return true if the string contains only numbers.
  * `string::starts_with(string, start)`: Returns true if the string starts with the specified substring.
  * `string::ends_with(string, end)`: Returns true if the string ends with the specified substring.
  * `string::replace(string, what, with)`: Replaces the specified substring with another string.
  * `string::regex_replace(string, expr, with)`: Replaces the matching substring with another string.
  * `string::regex_match(string, expr)`: Determines if a regex matches the entire string and returns the matches as an array.
  * `string::regex_search(string, expr)`: Determines if a regex matches part of the string.
    ```gsc
    init()
    {
        assert(string::to_upper("hello") == "HELLO");
        assert(string::to_lower("HELLO") == "hello");
        assert(string::is_numeric("12345"));
        assert(string::starts_with("hello world", "hello"));
        assert(string::ends_with("hello world", "world"));
        assert(string::replace("hello", "e", "o") == "hollo");
        assert(string::regex_replace("hello", "h|e|l|o", "a") == "aaaaa");
        assert(string::regex_match("hello", "^hello$").size == 1);
        assert(string::regex_search("hello", "e|o"));
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
