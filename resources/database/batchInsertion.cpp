#include "batchInsertion.h"
#include "attributes.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
bool batchInsertDictWords(sqlite3* db, const std::string& filePath) {
    //std::cerr << "In batchInsertDictWords " << std::endl;
    std::ifstream file(filePath);
    if (!file.is_open()) {

        std::ofstream file(filePath);
        std::cerr << "Unable to open file: " << filePath << std::endl;
        //return false;
    }

    const char* sql = "INSERT INTO dictionary (word, frequency, partOfSpeech, time, source) VALUES (?, ?, ?, ?, ?);";
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
    std::string line;
    std::string word;
    int insertCount = 0;
    const int BATCH_SIZE = 1000;
    const char* source = "dict";
    //std::cerr << "Before getline " << std::endl;
    int lineNum = 0;
    if(filePath == "data/Dictionary.txt"){
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
    }else{
        while (std::getline(file, line)) {
            if(lineNum<28){
                lineNum++;
                continue;
            }
            //std::cerr << "In getline " << std::endl;
            std::vector<std::string> lineSave;
            std::stringstream ss(line);
            std::string info;
            //std::cerr << "before pushing words in line to vector " << std::endl;
            while(ss>>info){
                //printf("%s, \n", info.c_str());
                lineSave.push_back(info);
            }
            //std::cerr << "before assigning partSpeech" << std::endl;
            std::string partSpeech = lineSave[2]; 
            //std::cerr << "before assigning word" << std::endl;
            std::string word = lineSave[4];
            if (containSymbols(word)){
                continue;
            }
            sqlite3_bind_text(stmt, WORDTXT, word.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, FREQUENCY, 1);
            sqlite3_bind_text(stmt, PARTOFSPEECH, partSpeech.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, TIME, currentTime);
            sqlite3_bind_text(stmt, SOURCE, source, -1, SQLITE_STATIC);

            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                //std::cerr << "Insertion failed for word '" << word << "': " << sqlite3_errmsg(db) << std::endl;
            }

            sqlite3_reset(stmt);
            insertCount++;
            lineNum++;

            // Periodic commit to avoid huge transactions
            if (insertCount % BATCH_SIZE == 0) {
                sqlite3_exec(db, "COMMIT; BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
                //std::cout << "Inserted " << insertCount << " words..." << std::endl;
            }
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
bool batchInsertLemmaWords(sqlite3* db, const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::ofstream file(filePath);
        std::cerr << "Unable to open file: " << filePath << std::endl;
        //return false;
    }

    const char* sql = "INSERT INTO lemma (word, part-of-speech) VALUES (?, ?);";
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

    std::string line;
    int insertCount = 0;
    const int BATCH_SIZE = 1000;

    while (std::getline(file, line)) {
        std::vector<std::string> lineSave;
        std::stringstream ss(line);
        std::string info;
        while(ss>>info){
            lineSave.push_back(info);
        }
        std::string word = lineSave[4]; 
        std::string partSpeech = lineSave[2]; 
        if (containSymbols(word)){
            continue;
        }
        sqlite3_bind_text(stmt, LEMMAWORDTXT, word.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, PARTOFSPEECH, partSpeech.c_str(), -1, SQLITE_STATIC);

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
bool batchInsertInflectionWords(sqlite3* db, const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::ofstream file(filePath);
        std::cerr << "Unable to open file: " << filePath << std::endl;
        //return false;
    }

    const char* sql = "INSERT INTO lemma (word, part-of-speech) VALUES (?, ?);";
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

    std::string line;
    int insertCount = 0;
    const int BATCH_SIZE = 1000;

    while (std::getline(file, line)) {
        std::vector<std::string> lineSave;
        std::stringstream ss(line);
        std::string info;
        while(ss>>info){
            lineSave.push_back(info);
        }
        std::string word = lineSave[4]; 
        std::string partSpeech = lineSave[2]; 
        if (containSymbols(word)){
            continue;
        }
        sqlite3_bind_text(stmt, LEMMAWORDTXT, word.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, PARTOFSPEECH, partSpeech.c_str(), -1, SQLITE_STATIC);

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