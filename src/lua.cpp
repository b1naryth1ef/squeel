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

void stackDump (lua_State *L) {
    int i;
    int top = lua_gettop(L);
    for (i = 1; i <= top; i++) {  /* repeat for each level */
      int t = lua_type(L, i);
      switch (t) {

        case LUA_TSTRING:  /* strings */
          printf("`%s'", lua_tostring(L, i));
          break;

        case LUA_TBOOLEAN:  /* booleans */
          printf(lua_toboolean(L, i) ? "true" : "false");
          break;

        case LUA_TNUMBER:  /* numbers */
          printf("%g", lua_tonumber(L, i));
          break;

        default:  /* other values */
          printf("%s", lua_typename(L, t));
          break;

      }
      printf("  ");  /* put a separator */
    }
    printf("\n");  /* end the listing */
  }


