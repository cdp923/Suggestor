#include "resources/keyboard.h"
#include "resources/database/methods.h" 
#include "resources/database/attributes.h"
#include "autoSuggest.h"
#include <iostream>
#include <sstream>
#include <fstream>
int main(){
    sqlite3* db;
    std::vector<std::vector<char>> keyGraph = initKeyboard();
    std::string input;
    if(!initializeDB(db, "dictionary", "data/my_dictionary.db")){
        return false;
    }
    printf("Type: ");
    std::getline(std::cin, input);
    std::istringstream iss(input);
    std::string seperate;
    while ( getline( iss, seperate, ' ' ) ) {
        std::vector<std::string> words;
        std::string word;
        //std::vector<std::wstring> wordCorrectior = fullAutoCorrect(word, dictGraph, keyGraph);
        //std::vector<std::wstring> test= letterInsert(L"mixac");
        while (iss >> word) {
            words.push_back(word);
        }
        std::vector<std::string> wordCorrectior = autoSuggest(db, word, keyGraph);
        //std::vector<std::wstring> wordCorrectior = simpleAutoCorrect(word, dictGraph, keyGraph);
        //std::vector<std::wstring> test= letterSwap(L"systen");
    }
    //*/
/*
cd ..
cd ..
cd projects/corrector
g++ autoSuggest.cpp autocomplete.cpp  resources/keyboard.cpp resources/algorithms.cpp resources/wordCombos.cpp resources/database/methods.cpp resources/database/attributes.cpp -x c resources/database/sqlite/sqlite3.c -o test.exe
*/
    return 0;
}