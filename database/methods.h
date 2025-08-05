#include "attributes.h"

bool createDictTable(sqlite3* db);
bool dbInsert(sqlite3* db,sqlite3_stmt* stmt, const char* sql, const WordData &wordData);
bool dbUpdate(sqlite3* db,const WordData &wordData);
bool initializeDB(sqlite3*& db, const char* dbName, const std::string &filePath);
void printDB(sqlite3*& db);

