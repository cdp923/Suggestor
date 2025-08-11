#include "methods.h"
#include "attributes.h"
#include <fstream>
#include <iostream>

bool createDictTable(sqlite3* db){
    const char* table = "CREATE TABLE IF NOT EXISTS dictionary("
                        "word TEXT PRIMARY KEY NOT NULL,"
                        "frequency INT DEFAULT 1,"
                        "time INT NOT NULL,"
                        "source TEXT NOT NULL);";
                        char* zErrMsg = 0;
    int rc = sqlite3_exec(db, table, nullptr, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error creating table: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    } else {
        std::cout << "Table dictionary created or already exists." << std::endl;
    }
    return true;
}
bool deleteWord(sqlite3* db, const WordData &wordData){
    sqlite3_stmt* stmt;
    const char* sql = "DELETE FROM dictionary WHERE word = ?;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    std::vector<std::string> attributes;
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << std::endl;
        return false; 
    }
    sqlite3_bind_text(stmt, WORDTXT, wordData.word.c_str(), -1, SQLITE_TRANSIENT);
    bool result = (sqlite3_step(stmt)== SQLITE_DONE);
    if (result != SQLITE_OK) {
        std::cerr << "Failed to delete: " << sqlite3_errmsg(db) << std::endl;
        return false; 
    }
    sqlite3_finalize(stmt);
    return result;
}
bool dbInsert(sqlite3* db, const WordData &wordData){
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO dictionary (word, frequency, time, source) VALUES (?, ?, ?, ?);";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (containSymbols(wordData.word)){
        return false;
    }
    sqlite3_bind_text(stmt, WORDTXT, wordData.word.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, FREQUENCY, 1); 
    sqlite3_bind_int(stmt, TIME, wordData.time); 
    sqlite3_bind_text(stmt, SOURCE, wordData.source.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Insertion failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}
bool batchInsertWords(sqlite3* db, const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::ofstream file(filePath);
        std::cerr << "Unable to open file: " << filePath << std::endl;
        //return false;
    }

    const char* sql = "INSERT INTO dictionary (word, frequency, time, source) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        file.close();
        return false;
    }

    rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to begin transaction: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        file.close();
        return false;
    }

    time_t currentTime = getCurrentTime();
    std::string word;
    int insertCount = 0;
    const int BATCH_SIZE = 1000;
    const char* source = "dict";

    while (file >> word) {
        if (containSymbols(word)){
            continue;
        }
        sqlite3_bind_text(stmt, WORDTXT, word.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, FREQUENCY, 1);
        sqlite3_bind_int(stmt, TIME, currentTime);
        sqlite3_bind_text(stmt, SOURCE, source, -1, SQLITE_STATIC);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            //std::cerr << "Insertion failed for word '" << word << "': " << sqlite3_errmsg(db) << std::endl;
        }

        sqlite3_reset(stmt);
        insertCount++;

        // Periodic commit to avoid huge transactions
        if (insertCount % BATCH_SIZE == 0) {
            sqlite3_exec(db, "COMMIT; BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
            //std::cout << "Inserted " << insertCount << " words..." << std::endl;
        }
    }

    // Final commit
    rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to commit transaction: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        file.close();
        return false;
    }

    sqlite3_finalize(stmt);
    file.close();
    std::cout << "Successfully inserted " << insertCount << " words total." << std::endl;
    return true;
}
bool dbUpdate(sqlite3* db,const WordData &wordData){
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE OR IGNORE dictionary SET frequency=?, time=? WHERE word = ?;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        return false;
    }
    int frequency = getWordFrequency(db, wordData);
 
    sqlite3_bind_int(stmt, FREQUENCY, frequency+1); 
    sqlite3_bind_int(stmt, TIME, wordData.time); 
    sqlite3_bind_text(stmt, WORDTXT, wordData.word.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Insertion failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool initializeDB(sqlite3*& db, const char* dbName, const std::string& filePath){
    int rc = sqlite3_open(dbName, &db);

    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    if (!createDictTable(db)) {
        sqlite3_close(db);
        return false;
    }

    if (!batchInsertWords(db, filePath)) {
        sqlite3_close(db);
        return false;
    }

    return true;
}
void printDB(sqlite3*& db){
    sqlite3_stmt* stmt;
    const char* sql = " SELECT word, frequency, time, source from dictionary ORDER BY word;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparing select statement for viewing: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    std::cout << "\n--- Contents of 'dictionary' table ---" << std::endl;

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    } 
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char* word = (const char*)sqlite3_column_text(stmt, WORDTXT-1);
        const char* source = (const char*)sqlite3_column_text(stmt, SOURCE-1);
        
        int frequency = sqlite3_column_int(stmt, FREQUENCY-1);
        sqlite3_int64 time_val = sqlite3_column_int64(stmt, TIME-1);

        std::cout << "Word = " << (word ? word : "NULL") << " | ";
        std::cout << "Frequency = " << frequency << " | ";
        std::cout << "Time = " << time_val << " | ";
        std::cout << "Source = " << (source ? source : "NULL") << std::endl;
    }
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Error during select step: " << sqlite3_errmsg(db) << std::endl;
        }
    
}