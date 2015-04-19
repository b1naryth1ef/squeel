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
      ref INTEGER,
      user INTEGER,
      msg TEXT
    );
  ]]
end

function types()
  return {1, 1, 3}
end

format = '"(.+)";"(.+)";"(.+)"'

function parse(line)
  ref, user, msg = string.match(line, format)
  return {tonumber(ref), tonumber(user), msg}
end
