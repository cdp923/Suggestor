#include "resources/database/sqlite/sqlite3.h"
#include <vector>
#include <string>

std::vector<std::string> autoComplete(sqlite3* db, std::string input);
