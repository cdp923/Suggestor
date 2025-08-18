#include "sqlite/sqlite3.h"
#include <fstream>
#include <string>

bool createProcessedFilesTable(sqlite3* db);
bool checkIfFileIsProcessed(sqlite3* db, const std::string& filePath);
bool logProcessedFile(sqlite3* db, const std::string& filePath);