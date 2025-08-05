#include "sqlite/sqlite3.h"
#include <vector>
#include <string>
#include <ctime>

struct WordData {
    std::string word;
    int time;
    std::string source; 
};
enum ColumnIndex {
    WORD = 1,
    FREQUENCY,
    TIME,
    SOURCE
};

time_t getCurrentTime();
int getWordFrequency(sqlite3* db,const WordData &wordData);
std::vector<std::string> getWordAttributes(sqlite3* db, const WordData &wordData);
bool wordExists(sqlite3* db, std::vector<std::string> wordCombos);
bool dictExists(sqlite3* db);

const std::string& tableName = "dictionary"; //change createDictTable if you change this