#pragma once
#include "global.h"

#include "lua.h"

sqlite3_module *createVirtualTableModule();

struct VirtualTable {
  public:
    std::string setup(sqlite3*);

    sqlite3_vtab *vtab;
    LuaFile *lua;
};

