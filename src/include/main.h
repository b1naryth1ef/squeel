#pragma once
#include "global.h"
SQLITE_EXTENSION_INIT1

extern "C" {
  int sqlite3_extension_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi);
}

