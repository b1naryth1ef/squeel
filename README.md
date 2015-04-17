# squeel
Squeel is a SQLite virtual table extension which adds support for dynamically querying flat-files on disk, using Lua scripting as a bridge.

## Example
_This example is very much not working. Development is in progress, and this is the desired end-goal of the project._
The default example assumes you want to use SQLite to query a normal syslog output of:
```
Apr 17 02:41:01 eos sm-msp-queue[8920]: unable to qualify my own domain name (eos) -- using short name
```

The initial step of querying a file is creating a lua module which will serve as a bridge between SQLite and the file system.

```lua
-- Setup allows for file introspection/etc before setting up the initial file
function setup(a, b, c)
  if a == nil or b == nil or c == nil then
    return "Arguments not set!"
  end

  print("Hello from lua!")
  return nil
end

-- Schema defines the actual table schema
function schema()
  return [[
    CREATE TABLE a (
      line INTEGER,
      date TEXT,
      host TEXT,
      msg TEXT
    );
  ]]
end

-- parse_line takes a single line and attempts to parse it into a SQLite row
function parse_line(line)
  return {1, "test", "test", "test"}
end
```

Finally, in sqlite:
```
SQLite version 3.8.2 2013-12-06 14:53:30
Enter ".help" for instructions
Enter SQL statements terminated with a ";"
sqlite> .load ./squeel
sqlite> CREATE VIRTUAL TABLE test USING squeel(/var/log/syslog, test.lua, 1, 2, 3);
Hello from lua!
sqlite> SELECT * FROM test WHERE host='eos';
...
```

