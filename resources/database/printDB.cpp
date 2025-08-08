#include "methods.h" 
#include "attributes.h"
#include <iostream>       
#include <string>         

int main() {
    sqlite3* myDatabase = nullptr; 
    const char* dbFilePath = "my_dictionary.db"; 
    const std::string textFilePath = "Dictionary.txt"; 

    initializeDB(myDatabase, dbFilePath, textFilePath);

    if (myDatabase) {
        std::cout << "\nDatabase initialization complete. Now performing a sample SELECT to print contents." << std::endl;

        char* zErrMsg = nullptr; 
        sqlite3_stmt* stmt = nullptr; 

        const char* select_sql = "SELECT word, frequency, time, source FROM dictionary LIMIT 5;";

        int rc = sqlite3_prepare_v2(myDatabase, select_sql, -1, &stmt, nullptr);

        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(myDatabase) << std::endl;
        } else {
            std::cout << "\n--- Sample Data from Dictionary ---" << std::endl;
           
        printDB(myDatabase);
        }
        
        sqlite3_finalize(stmt);

        sqlite3_close(myDatabase);
        std::cout << "Database closed." << std::endl;
    } else {
        std::cerr << "Database initialization failed. No database connection was established." << std::endl;
    }

    return 0; 
}
/*
cd ..
cd ..
cd projects/textEditorAutoCorrectComplete
g++ autoSuggest.cpp autocomplete.cpp  resources/keyboard.cpp resources/algorithms.cpp resources/wordCombos.cpp-o keyboard.exe
g++ database/methods.cpp -x c database/sqlite/sqlite3.c -o printDB.exe
printDB.exe
*/