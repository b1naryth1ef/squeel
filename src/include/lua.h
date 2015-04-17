#pragma once
#include "global.h"

#include <lua.hpp>

class LuaFile {
  public:
    std::string path;
    lua_State   *L;

    LuaFile(std::string path);
    ~LuaFile();

    bool open();
};

