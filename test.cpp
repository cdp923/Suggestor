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
    if(!initializeDB(db, "data/my_dictionary.db", "data/Dictionary.txt")){
        return -1;
    }
    printf("Type: ");
    std::getline(std::cin, input);
    std::istringstream iss(input);
    std::string seperate;
    while ( getline( iss, seperate, ' ' ) ) {
        std::vector<std::string> wordCorrectior = autoSuggest(db, seperate, keyGraph);
        //std::vector<std::string> wordCompleter = autoComplete(db, seperate);
    }
    sqlite3_close(db);
/*
cd ..
cd ..
cd projects/corrector
g++ test.cpp autoSuggest.cpp autocomplete.cpp  resources/keyboard.cpp resources/algorithms.cpp resources/wordCombos.cpp resources/database/methods.cpp resources/database/attributes.cpp -x c resources/database/sqlite/sqlite3.c -o test.exe
test.exe
*/
    return 0;
}
//bkjfbealjelkajbfelkjakjebflbjfpieuabfjPFUNPAESUNnfap