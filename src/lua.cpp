#include "include/lua.h"

LuaFile::LuaFile(std::string path) {
  this->path = path;
  this->L = luaL_newstate();
  luaL_openlibs(L);
}

bool LuaFile::open() {
  return luaL_dofile(L, path.c_str()) == 0;
}

LuaFile::~LuaFile() {
  lua_close(L);
}

