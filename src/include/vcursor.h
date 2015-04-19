#pragma once
#include "global.h"

#include <vector>

struct VirtualCursor : sqlite3_vtab_cursor {
  int line;
  int mode;

  // Represents all lines that match query
  std::vector<int> matches;

  void seek(int);
};
