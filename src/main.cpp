#include "include/main.h"
#include "include/vtable.h"

int sqlite3_extension_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi){
  SQLITE_EXTENSION_INIT2(pApi);

  return sqlite3_create_module(db, "squeel", createVirtualTableModule(), 0);
}
