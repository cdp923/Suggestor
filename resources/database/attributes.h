#include "sqlite/sqlite3.h"
#include <vector>
#include <string>
#include <ctime>
#ifndef ATTRIBUTES
#define ATTRIBUTES
#define MAXSUGGESTIONS 3
struct WordData {
    std::string word;
    int time;
    std::string source; 
};
enum DicColumnIndex {
    WORDTXT = 1,
    FREQUENCY,
    PARTOFSPEECH,
    TIME,
    SOURCE
};
enum LemmaColumnIndex {
    LEMMAWORDTXT = 1
};
extern const std::string tableName; //change createDictTable if you change this
#endif  

bool containSymbols(std::string input);
time_t getCurrentTime();
float getWordFrequency(sqlite3* db, const std::string& word);
std::vector<std::string> getWordAttributes(sqlite3* db, const WordData &wordData);
bool wordExists(sqlite3* db, std::vector<std::string>& wordCombos, std::vector<std::string>& comboSave);//word combos
bool wordExists(sqlite3* db, std::string& word); //single word
bool dictExists(sqlite3* db);
std::vector<std::string> getWordsStartingWith(sqlite3* db, char firstLetter, int length);
std::vector<std::string> getSubset(sqlite3* db, std::string prefix);
int noPoSNum(sqlite3* db);

