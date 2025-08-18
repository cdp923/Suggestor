#include "sqlite/sqlite3.h"
#include <string>
bool batchInsertDictWords(sqlite3* db, const std::string& filePath);
bool batchInsertLemmaWords(sqlite3* db, const std::string& filePath);
