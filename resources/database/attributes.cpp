#include "attributes.h"
#include <iostream>
#include <chrono>

bool containSymbols(std::string input){
    for (char c : input) {
        if (!std::isalpha(static_cast<unsigned char>(c))) {
            return true;
        }
    }
    return false;
}

time_t getCurrentTime(){
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t timeValue = std::chrono::system_clock::to_time_t(now);
    return timeValue;
}

float getWordFrequency(sqlite3* db, const std::string &word){
    sqlite3_stmt* stmt;
    const char* sql = "SELECT frequency FROM dictionary WHERE word = ?;";
    float frequency = 0.0;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        std::cerr << "Failed to prepare select statement in getWordFrequency: " << sqlite3_errmsg(db) << std::endl;
        return frequency; 
    }
    
    sqlite3_bind_text(stmt, 1, word.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        frequency = sqlite3_column_double(stmt,0);
    }
    sqlite3_finalize(stmt);
    return frequency;
}

std::vector<std::string> getWordAttributes(sqlite3* db, const WordData &wordData){
    sqlite3_stmt* stmt;
    const char* sql = "SELECT frequency,  partOfSpeech, time, source FROM dictionary WHERE word = ?;";
    std::vector<std::string> attributes;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << std::endl;
        return attributes; 
    }

    sqlite3_bind_text(stmt, WORDTXT, wordData.word.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        attributes.push_back(wordData.word); 
        attributes[WORDTXT-1] = wordData.word;
        attributes[FREQUENCY-1] = sqlite3_column_double(stmt,FREQUENCY);

        attributes[PARTOFSPEECH-1] = "NULL";
        
        attributes[TIME-1] = sqlite3_column_int(stmt,TIME);
        attributes[SOURCE-1] = sqlite3_column_int(stmt,SOURCE);
    }
    sqlite3_finalize(stmt);
    return attributes;
}

bool wordExists(sqlite3* db, std::string& word){
    sqlite3_stmt* stmt;
    const char* sql = "SELECT EXISTS(SELECT 1 FROM dictionary WHERE word = ?);";
    bool exists = false;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        std::cerr << "Failed to prepare select statement in wordExists: " << sqlite3_errmsg(db) << std::endl;
        return exists; 
    }

    sqlite3_bind_text(stmt, 1, word.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // The first column of the result set is the boolean value (0 or 1)
        exists = (sqlite3_column_int(stmt, 0) == 1);
    }
    sqlite3_finalize(stmt);
    return exists;
}

bool dictExists(sqlite3* db){
    sqlite3_stmt* stmt;
    const char* sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='dictionary';";
    bool exists = false;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement in dictExists" << ": " << sqlite3_errmsg(db) << std::endl;
        if (db) {
            sqlite3_close(db);
        }
        return exists;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // A row returned, so the table exists
        exists = true;
    }

    sqlite3_finalize(stmt);
    return exists;
} 
std::vector<std::string> getWordsStartingWith(sqlite3* db, char firstLetter, int length){
    std::vector<std::string> words;
    sqlite3_stmt* stmt;
   
    const char* sql = "SELECT word FROM dictionary WHERE word LIKE ? || '%' "
                     "AND LENGTH(word) BETWEEN ? - 2 AND ? + 2 "
                     "ORDER BY frequency DESC, word;";
   
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare select statement in getWordsStartingWith: " << sqlite3_errmsg(db) << std::endl;
        return words;
    }
   
    std::string letterStr(1, firstLetter);
    sqlite3_bind_text(stmt, 1, letterStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, length);
    sqlite3_bind_int(stmt, 3, length);
   
    while ((sqlite3_step(stmt)) == SQLITE_ROW) {
        const char* word = (const char*)sqlite3_column_text(stmt, 0);
        if (word) {
            words.push_back(std::string(word));
        }
    }
   
    sqlite3_finalize(stmt);
    return words;
}
std::vector<std::string> getSubset(sqlite3* db, std::string prefix){
    std::vector<std::string> words;
    sqlite3_stmt* stmt;
    
    const char* sql = "SELECT word FROM dictionary WHERE word LIKE ? || '%' ORDER BY frequency DESC, word;";
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << std::endl;
        return words;
    }
    
    sqlite3_bind_text(stmt, 1, prefix.c_str(), -1, SQLITE_TRANSIENT);
    //printf("Words starting with %c", firstLetter," : \n");
    printf("Suggested words: ");
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char* word = (const char*)sqlite3_column_text(stmt, 0);
        if (word != nullptr && (words.size()<MAXSUGGESTIONS)) {
            words.push_back(std::string(word));
            //std::cout << word << ", ";
        }
    }
    printf("\n");
    
    sqlite3_finalize(stmt);
    return words;
}
int noPoSNum(sqlite3* db){
    sqlite3_stmt* stmt;
    const char* sql = "SELECT COUNT(*) FROM dictionary WHERE partOfSpeech IS NULL";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare noPoSNum select statement: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }
    int count = 0;
    if(sqlite3_step(stmt)==SQLITE_ROW){
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}