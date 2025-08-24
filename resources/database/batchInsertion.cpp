#include "batchInsertion.h"
#include "attributes.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
int BATCH_SIZE = 1000; 
int batchDictInsertHelper(sqlite3* db, sqlite3_stmt* stmt, std::ifstream& file, int rc){
    int insertCount = 0;
    std::string word;
    time_t currentTime = getCurrentTime();

    while (file >> word) {
        for (char &c : word) { // Iterate by reference to modify the string in-place
            c = std::tolower(static_cast<unsigned char>(c)); // Cast to unsigned char for safe use with std::tolower
        }
        if (containSymbols(word)|| word.length() <=1){
            continue;
        }
        sqlite3_bind_text(stmt, WORDTXT, word.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, FREQUENCY, 1);
        sqlite3_bind_int(stmt, TIME, currentTime);
        sqlite3_bind_text(stmt, SOURCE, "dict", -1, SQLITE_STATIC);

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
    return insertCount;
}
int batchPrinceDictInsertHelper(sqlite3* db, sqlite3_stmt* stmt, std::ifstream& file, int rc){
    time_t currentTime = getCurrentTime();
    std::string line;
    int insertCount = 0;
    int lineNum = 0;
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
        std::string partSpeech = lineSave[2]; 
        //std::cerr << "before assigning word" << std::endl;
        std::string wordSpace = lineSave[4];
        std::vector<std::string> wordSave;
        std::string segment;
        std::stringstream wordSpliter(wordSpace);
        while(std::getline(wordSpliter, segment, '_'))
        {
           wordSave.push_back(segment);
        }
        for(int i = 0; i<wordSave.size(); i++){
            std::string word = wordSave[i];
            if (containSymbols(word)){
                continue;
            }
            sqlite3_bind_text(stmt, WORDTXT, word.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, FREQUENCY, 1);
            sqlite3_bind_text(stmt, PARTOFSPEECH, partSpeech.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, TIME, currentTime);
            sqlite3_bind_text(stmt, SOURCE, "dict", -1, SQLITE_STATIC);

            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                //std::cerr << "Insertion failed for word '" << word << "': " << sqlite3_errmsg(db) << std::endl;
            }

            sqlite3_reset(stmt);
            insertCount++;
            lineNum++;
        }
        // Periodic commit to avoid huge transactions
        if (insertCount % BATCH_SIZE == 0) {
            sqlite3_exec(db, "COMMIT; BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
            //std::cout << "Inserted " << insertCount << " words..." << std::endl;
        }
    }
    return insertCount;
}
int batchPoSDictInsertHelper(sqlite3* db, sqlite3_stmt* stmt, std::ifstream& file, int rc){
    time_t currentTime = getCurrentTime();
    std::string line;
    std::string partSpeech;
    int insertCount = 0;
    int lineNum = 0;
    //std::cerr << "here " << std::endl;
    while (std::getline(file, line)) {
        //std::cerr << "In getline " << std::endl;
        std::vector<std::string> lineSave;
        std::stringstream ss(line);
        std::string info;
        //std::cerr << "here " << std::endl;
        while(ss>>info){
            //printf("%s, \n", info.c_str());
            lineSave.push_back(info);
        }
        if(lineNum == 0 ){
            partSpeech = lineSave[1]; 
            lineNum++;
            continue;
        }
        //std::cerr << "before assigning partSpeech" << std::endl;
        //std::cerr << "before assigning word" << std::endl;
        std::string word = lineSave[0];
        if (containSymbols(word)){
            continue;
        }
        sqlite3_bind_text(stmt, WORDTXT, word.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, FREQUENCY, 1);
        sqlite3_bind_text(stmt, PARTOFSPEECH, partSpeech.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, TIME, currentTime);
        sqlite3_bind_text(stmt, SOURCE, "dict", -1, SQLITE_STATIC);

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
    return insertCount;
}
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
    std::string word;
    std::string line;
    int insertCount = 0;
    const char* source = "dict";
    //std::cerr << "Before getline " << std::endl;
    int lineNum = 0;
    if(filePath.rfind("data/princetonDict", 0) == 0){
        insertCount+=batchPrinceDictInsertHelper(db, stmt, file, rc);
    }else if(filePath.rfind("data/PoS", 0) == 0){
        insertCount+=batchPoSDictInsertHelper(db, stmt, file, rc);
    }else{
        insertCount+=batchDictInsertHelper(db, stmt, file, rc);
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