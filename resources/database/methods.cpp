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
                        "frequency DOUBLE DEFAULT 0.000000003,"
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
bool createTables(sqlite3* db){
    if (!createProcessedFileTable(db)) {
        sqlite3_close(db);
        return false;
    }
    if (!createDictTable(db)) {
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
    int frequency = getWordFrequency(db, wordData.word);
 
    sqlite3_bind_double(stmt, FREQUENCY, frequency*1.001); 
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
bool bulkUpdateWordfreqData(sqlite3* db, const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open wordfreq data file at " << filePath << std::endl;
        return false;
    }

    std::string line;
    // Read and ignore the header row
    std::getline(file, line);

    sqlite3_stmt* stmt;
    const char* sql = "UPDATE OR IGNORE dictionary SET frequency = ? WHERE word = ?;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare update statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);

    int count = 0;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string word_str, freq_str;

        if (std::getline(ss, word_str, ',') && std::getline(ss, freq_str, ',')) {
            double frequency = std::stod(freq_str);
            
            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);

            sqlite3_bind_double(stmt, 1, frequency);
            sqlite3_bind_text(stmt, 2, word_str.c_str(), -1, SQLITE_TRANSIENT);

            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                //expected if the word wasn't in table
                std::cerr << "Update failed for word '" << word_str << "': " << sqlite3_errmsg(db) << std::endl;
            }
            count++;
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", 0, 0, 0);
    
    std::cout << "Attempted to update " << count << " words from wordfreq_data.csv." << std::endl;
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
    sqlite3_bind_double(stmt, FREQUENCY, 1); 
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
bool initializeDB(sqlite3*& db, const char* dbName){
    std::vector<std::string>WordNetFiles = {
        "data/princetonDict/data.adj", "data/princetonDict/data.adv", "data/princetonDict/data.verb",
        "data/princetonDict/data.noun", "data/PoS/prepositions.txt", "data/PoS/pronouns.txt", "data/PoS/interjections.txt", 
        "data/PoS/conjunctions.txt","data/Dictionary.txt", "data/drugs.txt","data/explicit.txt"
    };
    const std::string freqFile = "data/wordfreq_data.csv";
    int rc = sqlite3_open(dbName, &db);

    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    createTables(db);
    for(int fileIndex = 0; fileIndex<WordNetFiles.size(); fileIndex++){
        std::string filePath = WordNetFiles[fileIndex];
        if(!checkIfFileIsProcessed(db, filePath)){
            printf("File not processed\n");
            if (!batchInsertDictWords(db, filePath)) {
                sqlite3_close(db);
                return false;
            }
            logProcessedFile(db, filePath);
        }
    }
    if(!checkIfFileIsProcessed(db, freqFile)){
        printf("File not processed\n");
        if (!bulkUpdateWordfreqData(db, freqFile)) {
            sqlite3_close(db);
            return false;
        }
        logProcessedFile(db, freqFile);
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
        
        int frequency = sqlite3_column_double(stmt, FREQUENCY-1);
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

    //std::cout << "\n--- Contents of 'dictionary' table ---" << std::endl;

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