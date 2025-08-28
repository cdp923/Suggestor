#include "sqlite/sqlite3.h"
#include "attributes.h"
#include <vector>
#include <string>

bool createDictTable(sqlite3* db);
bool createLemmaTable(sqlite3* db);
bool createInflectionTable(sqlite3* db);
bool dbInsert(sqlite3* db,sqlite3_stmt* stmt, const WordData &wordData);
bool dbUpdate(sqlite3* db,const WordData &wordData);
bool initializeDB(sqlite3*& db, const char* dbName);
void printDB(sqlite3*& db);
void checkWord(sqlite3*& db);
