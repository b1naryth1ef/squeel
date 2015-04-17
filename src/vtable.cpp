#include "include/vtable.h"

#include <functional>

static int iVersion = 0;

std::string VirtualTable::setup(sqlite3 *db) {
  // Call the schema function
  lua_getglobal(lua->L, "schema");
  if (lua_pcall(lua->L, 0, 1, 0) != 0) {

    return std::string(lua_tostring(lua->L, -1));
  }

  char *result = (char *)lua_tostring(lua->L, -1);
  if (result == 0) {
    return "Error parsing result";
  }

  this->vtab = new sqlite3_vtab;

  if (sqlite3_declare_vtab(db, result) != SQLITE_OK) {
    return "Error declaring virtual table";
  }
  return "";
}

int xCreate(sqlite3 *db, void *paux, int argc, const char* const *argv, sqlite3_vtab **vtab, char **pzErr) {
  if (argc < 5) {
    *pzErr = sqlite3_mprintf("Missing file path and module path arguments.");
    return SQLITE_ERROR;
  }

  // First, load our lua source file
  VirtualTable *vt = new VirtualTable;
  vt->lua = new LuaFile(argv[4]);

  // Attempt to load the lua module
  if (!vt->lua->open()) {
    *pzErr = sqlite3_mprintf("Error loading module file: %s",
        lua_tostring(vt->lua->L, -1));
    return SQLITE_ERROR;
  }

  // Push setup function onto stack
  lua_getglobal(vt->lua->L, "setup");

  // Push optional arguments onto stack
  for (int i = 5; i < argc; i++) {
    lua_pushstring(vt->lua->L, argv[i]);
  }

  // Attempt to call the setup function, returning the lua VM error if it fails
  if (lua_pcall(vt->lua->L, argc - 5, 1, 0) != 0) {
    *pzErr = sqlite3_mprintf("Error calling module setup function: %s",
        lua_tostring(vt->lua->L, -1));
    return SQLITE_ERROR;
  }

  // If the setup function returns a valid string (e.g. not null) we have an error
  char *result = (char *)lua_tostring(vt->lua->L, -1);
  if (result != 0) {
    *pzErr = sqlite3_mprintf("Error calling module setup function: %s", result);
    return SQLITE_ERROR;
  }

  std::string err = vt->setup(db);
  if (err != "") {
    *pzErr = sqlite3_mprintf("Error setting up schema: %s", err.c_str());
    return SQLITE_ERROR;
  }

  *vtab = vt->vtab;

  return SQLITE_OK;
}

int xConnect(sqlite3 *db, void *paux, int argc, const char* const *argv, sqlite3_vtab **vtab, char **pzerr) {
  return 0;
}

int xBestIndex(sqlite3_vtab *vtab, sqlite3_index_info *info) {
  return 0;
}

int xDisconnect(sqlite3_vtab *vtab) {
  return 0;
}

int xDestroy (sqlite3_vtab *vtab) {
  return 0;
}

// Creates a virtual table with correctly bound values and function pointers
sqlite3_module *createVirtualTableModule() {
  sqlite3_module *mod = new(sqlite3_module);

  mod->iVersion = iVersion;
  mod->xCreate = &xCreate;
  mod->xBestIndex = &xBestIndex;
  mod->xDisconnect = &xDisconnect;
  mod->xDestroy = &xDestroy;

  return mod;
}


