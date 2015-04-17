#pragma once
#include "global.h"
#include "vcursor.h"
#include "lua.h"

#include <functional>
#include <iostream>
#include <fstream>
#include <ostream>

sqlite3_module *createVirtualTableModule();

enum IndexType {
  // The default index type is just scanning the entire file
  CURSOR_SCAN,

  CURSOR_ROWID_EQ,
  CURSOR_ROWID_GT,
  CURSOR_ROWID_LE,
  CURSOR_ROWID_LT,
  CURSOR_ROWID_GE
};

const long CURSOR_SCAN_COST = 10000.0;
const long CURSOR_ROWID_COST = 1.0;
const long CURSOR_RELATIVE_ROWID_COST = 2500.0;


struct VirtualTable : sqlite3_vtab{
  std::string setup(sqlite3*);

  LuaFile *lua;

  // Represents file information
  std::ifstream *file;
  long int length;

  std::string openFile(std::string);
};

