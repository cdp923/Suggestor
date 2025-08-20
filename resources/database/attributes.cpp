#include "attributes.h"
#include <iostream>
#include <chrono>
bool containSymbols(std::string input){
    int stringIndex =0;
    while (stringIndex<input.size()){
        if ((int)input[stringIndex]>122 || (int)input[stringIndex]<97){
            return true;

        }
        stringIndex++;
    }
    return false;
}
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
    const char* sql = "SELECT frequency,  partOfSpeech, time, source FROM dictionary WHERE word = ?;";
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

        attributes[PARTOFSPEECH-1] = '0'; //CHANGE THIS!!!!!
        
        attributes[TIME-1] = sqlite3_column_int(stmt,TIME);
        attributes[SOURCE-1] = sqlite3_column_int(stmt,SOURCE);
    } else if (rc == SQLITE_DONE) {
        // The word wasn't in the dictionary.
        std::cerr << "Error during select step for word '" << wordData.word.c_str() << "': " << sqlite3_errmsg(db) << std::endl;
        return attributes;
    }
    return attributes;
}

bool wordExists(sqlite3* db, std::vector<std::string>& wordCombos, std::vector<std::string>& comboSave){
    sqlite3_stmt* stmt;
    const char* sql = "SELECT EXISTS(SELECT 1 FROM dictionary WHERE word = ?);";
    for(int index = 0; index<wordCombos.size();index++){    
        std::string word = wordCombos[index];
        //std::cout << "Word: '" << word << "' (from word combos)" << std::endl;
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
                comboSave.push_back(word);
            } else {
                //std::cout << "The word '" << word << "' does not exist in the database." << std::endl;
                continue;
            }
        } else if (rc == SQLITE_DONE) {
            return false;
        } else {
            std::cerr << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
    }
    sqlite3_finalize(stmt);
    return false;
}
/*
bool wordExists(sqlite3* db, std::string& word){
    //std::cout << "Word: '" << word << "' (from single word)" << std::endl;
    if(word.size()<2){
        std::cout << "The word '" << word << "' exists in the database." << std::endl;
        return true;
    }
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
            std::cout << "The word '" << word << "' exists in the database." << std::endl;
            return true;
        } else {
            //std::cout << "The word '" << word << "' does not exist in the database." << std::endl;
            return false;
        }
    } else if (rc == SQLITE_DONE) {
        return false;
    } else {
        std::cerr << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_finalize(stmt);
    return false;
}
*/
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
std::vector<std::string> getWordsStartingWith(sqlite3* db, char firstLetter, int length){
    std::vector<std::string> words;
    sqlite3_stmt* stmt;
   
    const char* sql = "SELECT word FROM dictionary WHERE word LIKE ? || '%' "
                     "AND LENGTH(word) = ? "
                     "ORDER BY frequency DESC, word;";
   
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << std::endl;
        return words;
    }
   
    std::string letterStr(1, firstLetter);
    sqlite3_bind_text(stmt, 1, letterStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, length);
   
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
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
            //printf("%s, ", word,"\n");
            words.push_back(std::string(word));
            printf("%s, ",std::string(word).c_str());
        }
    }
    printf("\n");
    
    sqlite3_finalize(stmt);
    return words;
}