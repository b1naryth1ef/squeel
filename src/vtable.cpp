#include "include/vtable.h"


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

  if (sqlite3_declare_vtab(db, result) != SQLITE_OK) {
    return "Error declaring virtual table";
  }

  return "";
}

std::string VirtualTable::openFile(std::string name) {
  this->file = new std::ifstream;
  try {
    file->open(name, std::ios::in);
  } catch (std::string e) {
    return "Failed to open file: " + e;
  }

  char buffer[4096];
  while (this->file->getline(buffer, 4096)) {
    this->length++;
  }

  // Seek back to the start
  this->file->seekg(0, file->beg);
  this->pos = 0;

  return "";
}

std::string VirtualTable::getLine(int num) {
  char buffer[4096];

  printf("Length: %i\n", this->length);
  printf("%i, %i\n", num, this->pos);

  if (num == this->pos) {
    this->file->getline(buffer, 4096);
    return buffer;
  } else if (num > this->pos) {
    for (int i = this->pos; i < num; i++) {
      this->file->getline(buffer, 4096);
      this->pos++;
    }

    this->file->getline(buffer, 4096);
    return buffer;
  } else {
    printf("TODO LOL\n");
  }

  return "";
};

int xCreate(sqlite3 *db, void *paux, int argc, const char* const *argv, sqlite3_vtab **vtab, char **pzErr) {
  std::string err;

  if (argc < 5) {
    *pzErr = sqlite3_mprintf("Missing file path and module path arguments.");
    return SQLITE_ERROR;
  }

  // First, load our lua source file
  VirtualTable *vt = new VirtualTable;
  vt->lua = new LuaFile(argv[4]);

  err = vt->openFile(argv[3]);
  if (err != "") {
    *pzErr = sqlite3_mprintf("Error opening log file: %s", err.c_str());
    return SQLITE_ERROR;
  }

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

  err = vt->setup(db);
  if (err != "") {
    *pzErr = sqlite3_mprintf("Error setting up schema: %s", err.c_str());
    return SQLITE_ERROR;
  }

  // Finally, point our sqlite3_vtab reference to our new VirtualTable
  *vtab = vt;

  return SQLITE_OK;
}

int xConnect(sqlite3 *db, void *paux, int argc, const char* const *argv, sqlite3_vtab **vtab, char **pzerr) {
  return xCreate(db, paux, argc, argv, vtab, pzerr);
}

int xBestIndex(sqlite3_vtab *vtab, sqlite3_index_info *info) {
  info->idxNum = CURSOR_SCAN;
  info->estimatedCost = CURSOR_SCAN_COST;

  for (int i = 0; i < info->nConstraint; ++i) {
    sqlite3_index_info::sqlite3_index_constraint *cons = &info->aConstraint[i];
    if (!cons->usable) continue;

    if (cons->iColumn == -1) {
      info->aConstraintUsage->argvIndex = 1;
      info->estimatedCost = cons->op == SQLITE_INDEX_CONSTRAINT_EQ ? CURSOR_ROWID_COST : CURSOR_RELATIVE_ROWID_COST;

      switch(cons->op) {
        case SQLITE_INDEX_CONSTRAINT_EQ:
          info->idxNum = CURSOR_ROWID_EQ;
          break;
        case SQLITE_INDEX_CONSTRAINT_GT:
          info->idxNum = CURSOR_ROWID_GT;
          break;
        case SQLITE_INDEX_CONSTRAINT_LE:
          info->idxNum = CURSOR_ROWID_LE;
          break;
        case SQLITE_INDEX_CONSTRAINT_LT:
          info->idxNum = CURSOR_ROWID_LT;
          break;
        case SQLITE_INDEX_CONSTRAINT_GE:
          info->idxNum = CURSOR_ROWID_GE;
          break;
      }
      break;
    }
  }

  return SQLITE_OK;
}

int xDestroy(sqlite3_vtab *vtab) {
  delete((VirtualTable *) vtab);

  return SQLITE_OK;
}

int xDisconnect(sqlite3_vtab *vtab) {
  return xDestroy(vtab);
}


int xOpen(sqlite3_vtab *vtab, sqlite3_vtab_cursor **cursor) {
  *cursor = new VirtualCursor;

  return SQLITE_OK;
}

int xClose(sqlite3_vtab_cursor *cursor) {
  delete((VirtualCursor *) cursor);

  return SQLITE_OK;
}

int xEof(sqlite3_vtab_cursor *cursor) {
  VirtualCursor *c = (VirtualCursor *) cursor;
  VirtualTable *vtab = (VirtualTable *) c->pVtab;

  if (c->line <= vtab->length) {
    return 0;
  } else {
    return 1;
  }
}

int xFilter(sqlite3_vtab_cursor *c, int idxNum, const char *idxStr, int argc, sqlite3_value **argv) {
  VirtualCursor *cursor = (VirtualCursor *) c;
  cursor->mode = idxNum;

  if (idxNum == CURSOR_SCAN) {
    printf("Scanning table...\n");
    cursor->seek(0);
  }

  return SQLITE_OK;
}

int xNext(sqlite3_vtab_cursor *c) {
  VirtualCursor *cursor = (VirtualCursor *) c;
  if (cursor->mode == CURSOR_SCAN) {
    cursor->seek(cursor->line + 1);
  }
  return SQLITE_OK;
}

int xColumn(sqlite3_vtab_cursor *cursor, sqlite3_context*ctx, int N) {
  VirtualCursor *c = (VirtualCursor *) cursor;
  VirtualTable *vtab = (VirtualTable *) c->pVtab;

  // TODO cahe the fuck out of this
  std::string data = vtab->getLine(c->line);
  printf("DATA: '%s'\n", data.c_str());
  lua_getglobal(vtab->lua->L, "parse");
  lua_pushstring(vtab->lua->L, data.c_str());

  if (lua_pcall(vtab->lua->L, 1, 1, 0) != 0) {
    printf("FAILED TO PARSE LINE: %s\n",
        lua_tostring(vtab->lua->L, -1));
    return SQLITE_ERROR;
  }

  lua_pushnil(vtab->lua->L);

  int i = 0;
  while (lua_next(vtab->lua->L, -2) != 0) {
    if (i == N) {
      switch (lua_type(vtab->lua->L, -1)) {
        case LUA_TNUMBER: {
            double result = lua_tonumber(vtab->lua->L, -1);

            // Cast too integer if all digits after the decimal point are zero
            if ((result - (int) result) == result) {
              sqlite3_result_int(ctx, (int) result);
            } else {
              sqlite3_result_double(ctx, lua_tonumber(vtab->lua->L, -1));
            }

            lua_pop(vtab->lua->L, 1);
            return SQLITE_OK;
          }
        case LUA_TSTRING: {
            std::string result = lua_tostring(vtab->lua->L, -1);
            sqlite3_result_text(ctx, result.c_str(), -1, 0);
            lua_pop(vtab->lua->L, 1);
            return SQLITE_OK;
          }
        default:
          return SQLITE_ERROR;
      }
    }

    lua_pop(vtab->lua->L, 1);
    i++;
  }


  printf("%i %i\n", i, N);
  return SQLITE_ERROR;
}

int xRowid(sqlite3_vtab_cursor *c, sqlite_int64 *pRowid) {
  printf("Hi from rowid!\n");
  return 0;
}



// Creates a virtual table with correctly bound values and function pointers
sqlite3_module *createVirtualTableModule() {
  sqlite3_module *mod = new(sqlite3_module);

  mod->iVersion = iVersion;
  mod->xCreate = &xCreate;
  mod->xBestIndex = &xBestIndex;
  mod->xDisconnect = &xDisconnect;
  mod->xOpen = &xOpen;
  mod->xClose = &xClose;
  mod->xDestroy = &xDestroy;

  mod->xEof = &xEof;
  mod->xFilter = &xFilter;
  mod->xNext = &xNext;
  mod->xColumn = &xColumn;

  return mod;
}


