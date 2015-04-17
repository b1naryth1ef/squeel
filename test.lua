function setup(a, b, c)
  if a == nil or b == nil or c == nil then
    return "Arguments not set!"
  end

  print("Hello from lua!")
  return nil
end

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
