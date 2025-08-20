#include "resources/keyboard.h"
#include "resources/database/methods.h" 
#include "resources/database/attributes.h"
#include "autoSuggest.h"
#include "autocomplete.h"
#include <iostream>
#include <sstream>
#include <fstream>
int main(){
    sqlite3* db;
    std::vector<std::vector<char>> keyGraph = initKeyboard();
    std::string input;

    std::vector<std::string>WordNetFiles = {
        "data/princetonDict/data.adj","data/princetonDict/data.adv","data/princetonDict/data.verb",
        "data/princetonDict/index.adj","data/princetonDict/index.adv","data/princetonDict/index.noun", 
        "data/princetonDict/index.verb",
        "data/Dictionary.txt"
    };
    for(int fileIndex = 0; fileIndex<WordNetFiles.size(); fileIndex++){
        std::string filePath = WordNetFiles[fileIndex];
        if(!initializeDB(db, "data/my_dictionary.db", filePath)){
            printf("Initilization failed for %s", filePath);
            return -1;
        }
    }
    //printDB(db);
    checkWord(db);
    printf("Type: ");
    std::getline(std::cin, input);
    std::istringstream iss(input);
    std::string seperate;
    while ( getline( iss, seperate, ' ' ) ) {
        if (containSymbols(seperate)){
            continue;
        }
        std::vector<std::string> wordCorrectior = autoSuggest(db, seperate, keyGraph);
        //std::vector<std::string> wordCompleter = autoComplete(db, seperate);
    }
    sqlite3_close(db);
/*
cd ..
cd ..
cd projects/corrector
g++ test.cpp autoSuggest.cpp autocomplete.cpp  resources/keyboard.cpp resources/algorithms.cpp resources/wordCombos.cpp resources/database/methods.cpp resources/database/attributes.cpp resources/database/batchInsertion.cpp resources/database/fileProcessed.cpp -x c resources/database/sqlite/sqlite3.c -o test.exe
test.exe
*/
    return 0;
}
//bkjfbealjelkajbfelkjakjebflbjfpieuabfjPFUNPAESUNnfap