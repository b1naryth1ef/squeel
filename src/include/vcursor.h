#pragma once
#include "global.h"

#include <vector>

struct VirtualCursor : sqlite3_vtab_cursor {
  int line;

  // Represents all lines that match query
  std::vector<int> matches;
};
