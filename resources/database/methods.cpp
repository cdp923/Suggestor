#include "methods.h"
#include "attributes.h"
#include "batchInsertion.h"
#include "fileProcessed.h"
#include <fstream>
#include <iostream>
#include <sstream>

bool createDictTable(sqlite3* db){
    const char* table = "CREATE TABLE IF NOT EXISTS dictionary("
                        "word TEXT PRIMARY KEY NOT NULL,"
                        "frequency INT DEFAULT 1,"
                        "partOfSpeech TEXT DEFAULT NULL,"
                        "time INT NOT NULL,"
                        "source TEXT NOT NULL);";
    char* zErrMsg = 0;
    int rc = sqlite3_exec(db, table, nullptr, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error creating dictionary table: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    } else {
        //std::cout << "Table dictionary created or already exists." << std::endl;
    }
    return true;
}
bool createLemmaTable(sqlite3* db){
    const char* table = "CREATE TABLE IF NOT EXISTS lemma("
                        "word TEXT PRIMARY KEY NOT NULL,"
                        "partOfSpeech TEXT NOT NULL);";
    char* zErrMsg = 0;
    int rc = sqlite3_exec(db, table, nullptr, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error creating lemma table: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    } else {
        //std::cout << "Lemma table created or already exists." << std::endl;
    }
    return true;
}
bool createInflectionTable(sqlite3* db){
    const char* table = "CREATE TABLE IF NOT EXISTS inflection("
                        "id INT PRIMARY KEY,"
                        "form TEXT NOT NULL,"
                        "lemma TEXT NOT NULL,"
                        "tense TEXT,"
                        "FOREIGN KEY (lemma) REFERENCES words(lemma);";
    char* zErrMsg = 0;
    int rc = sqlite3_exec(db, table, nullptr, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error creating inflection table: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    } else {
        //std::cout << "Inflection table created or already exists." << std::endl;
    }
    return true;
}
bool createTables(sqlite3* db){
    if (!createProcessedFileTable(db)) {
        sqlite3_close(db);
        return false;
    }
    if (!createDictTable(db)) {
        sqlite3_close(db);
        return false;
    }
    if (!createLemmaTable(db)) {
        sqlite3_close(db);
        return false;
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

bool initializeDB(sqlite3*& db, const char* dbName, const std::string& filePath){
    int rc = sqlite3_open(dbName, &db);

    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    createTables(db);
    /*
    if (!createInflectionTable(db)) {
        sqlite3_close(db);
        return false;
    }
    */
    if(!checkIfFileIsProcessed(db, filePath)){
        printf("File not processed\n");
        if (!batchInsertDictWords(db, filePath)) {
            sqlite3_close(db);
            return false;
        }
        logProcessedFile(db, filePath);
        /*
        if (!batchInsertLemmaWords(db, filePath)) {
            sqlite3_close(db);
            return false;
        }
        */
    }

    return true;
}
void printDB(sqlite3*& db){
    sqlite3_stmt* stmt;
    const char* sql = " SELECT word, frequency, partOfSpeech, time, source FROM dictionary ORDER BY word;";
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
        const char* PoS = (const char*)sqlite3_column_text(stmt, PARTOFSPEECH-1);
        
        int frequency = sqlite3_column_int(stmt, FREQUENCY-1);
        sqlite3_int64 time_val = sqlite3_column_int64(stmt, TIME-1);

        std::cout << "Word = " << (word ? word : "NULL") << " | ";
        std::cout << "Frequency = " << frequency << " | ";
        std::cout << "Part of Speech = " << PoS << " | ";
        std::cout << "Time = " << time_val << " | ";
        std::cout << "Source = " << (source ? source : "NULL") << std::endl;
    }
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Error during select step: " << sqlite3_errmsg(db) << std::endl;
        }
    
}
void checkWord(sqlite3*& db){
    sqlite3_stmt* stmt;
    const char* sql = " SELECT word FROM dictionary WHERE word =  'theft';";
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
    }
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Error during select step: " << sqlite3_errmsg(db) << std::endl;
        }
}