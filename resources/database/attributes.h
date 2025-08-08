#include "sqlite/sqlite3.h"
#include <vector>
#include <string>
#include <ctime>
#ifndef ATTRIBUTES
#define ATTRIBUTES
struct WordData {
    std::string word;
    int time;
    std::string source; 
};
enum ColumnIndex {
    WORDTXT = 1,
    FREQUENCY,
    TIME,
    SOURCE
};
extern const std::string tableName; //change createDictTable if you change this
#endif  

time_t getCurrentTime();
int getWordFrequency(sqlite3* db,const WordData &wordData);
std::vector<std::string> getWordAttributes(sqlite3* db, const WordData &wordData);
bool wordExists(sqlite3* db, std::vector<std::string>& wordCombos);
bool wordExists(sqlite3* db, std::string& word);
bool dictExists(sqlite3* db);

