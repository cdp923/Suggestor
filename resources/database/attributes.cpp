#include "attributes.h"
#include <iostream>
#include <chrono>
time_t getCurrentTime(){
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t timeValue = std::chrono::system_clock::to_time_t(now);
    return timeValue;
}
int getWordFrequency(sqlite3* db,const WordData &wordData){
    sqlite3_stmt* stmt;
    const char* sql = "SELECT frequency FROM dictionary WHERE word = ?;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    int frequency = -1;
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << std::endl;
        return frequency; 
    }
    sqlite3_bind_text(stmt, WORDTXT, wordData.word.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to update: " << sqlite3_errmsg(db) << std::endl;
        return frequency; 
    }
    if (rc == SQLITE_ROW) {
        frequency = sqlite3_column_int(stmt,FREQUENCY);
    } else if (rc == SQLITE_DONE) {
        // The word wasn't in the dictionary.
        std::cerr << "Error during select step for word '" << wordData.word.c_str() << "': " << sqlite3_errmsg(db) << std::endl;
        frequency = -1; 
        return frequency;
    }
    return frequency;
}
std::vector<std::string> getWordAttributes(sqlite3* db, const WordData &wordData){
    sqlite3_stmt* stmt ;
    const char* sql = "SELECT frequency, time, source FROM dictionary WHERE word = ?;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    std::vector<std::string> attributes;
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << std::endl;
        return attributes; 
    }
    sqlite3_bind_text(stmt, WORDTXT, wordData.word.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind: " << sqlite3_errmsg(db) << std::endl;
        return attributes; 
    }
    if (rc == SQLITE_ROW) {
        attributes[WORDTXT-1] = wordData.word;
        attributes[FREQUENCY-1] = sqlite3_column_int(stmt,FREQUENCY);
        attributes[TIME-1] = sqlite3_column_int(stmt,TIME);
        attributes[SOURCE-1] = sqlite3_column_int(stmt,SOURCE);
    } else if (rc == SQLITE_DONE) {
        // The word wasn't in the dictionary.
        std::cerr << "Error during select step for word '" << wordData.word.c_str() << "': " << sqlite3_errmsg(db) << std::endl;
        return attributes;
    }
    return attributes;
}
bool wordExists(sqlite3* db, std::vector<std::string>& wordCombos){
    sqlite3_stmt* stmt;
    const char* sql = "SELECT EXISTS(SELECT 1 FROM dictionary WHERE word = ?);";
    for(int index = 0; index<wordCombos.size();index++){    
        std::string word = wordCombos[index];
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << std::endl;
            return false; 
        }
        sqlite3_bind_text(stmt, WORDTXT, word.c_str(), -1, SQLITE_TRANSIENT);
            rc = sqlite3_step(stmt);

        if (rc == SQLITE_ROW) {
            // The first column of the result set is the boolean value (0 or 1)
            int exists = sqlite3_column_int(stmt, 0);
            if (exists == 1) {
                return true;
            } else {
                std::cout << "The word '" << word << "' does not exist in the database." << std::endl;
                return false;
            }
        } else if (rc == SQLITE_DONE) {
            return false;
        } else {
            std::cerr << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return false;
}
bool wordExists(sqlite3* db, std::string& word){
    sqlite3_stmt* stmt;
    const char* sql = "SELECT EXISTS(SELECT 1 FROM dictionary WHERE word = ?);";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << std::endl;
        return false; 
    }
    sqlite3_bind_text(stmt, WORDTXT, word.c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        // The first column of the result set is the boolean value (0 or 1)
        int exists = sqlite3_column_int(stmt, 0);
        if (exists == 1) {
            return true;
        } else {
            std::cout << "The word '" << word << "' does not exist in the database." << std::endl;
            return false;
        }
    } else if (rc == SQLITE_DONE) {
        return false;
    } else {
        std::cerr << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return false;
}
bool dictExists(sqlite3* db){
    sqlite3_stmt* stmt;
    const char* sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='dictionary';";
    bool exists = false;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW) {
        std::cerr << "Failed to prepare statement" << ": " << sqlite3_errmsg(db) << std::endl;
        if (db) {
            sqlite3_close(db);
        }
        return exists;
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        // A row returned, so the table exists
        exists = true;
    }

    sqlite3_finalize(stmt);
    return exists;
} 