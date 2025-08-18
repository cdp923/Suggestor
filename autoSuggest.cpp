#include "autoSuggest.h"
#include "resources/algorithms.h"
#include "resources/wordCombos.h"
#include "resources/database/attributes.h"

#include <queue>

void closestWordSearch(std::vector<std::string>& combinations, std::vector<std::string>& closestResponses,
    std::vector<int>& closestResponsesDist, sqlite3* db, const std::vector<std::vector<char>>& keyGraph, 
    float& largestMinDist, float weight) {
    for (int index = 0; index < combinations.size(); index++) {
        std::string word = combinations[index];
        int stringIndex =0;
        if(word == ""){
            continue;
        }
        bool symbol = false;
        while (stringIndex<word.size()){
            if ((int)word[stringIndex]>122 || (int)word[stringIndex]<97){
                symbol = true;
            }
            stringIndex++;
        }
        if(symbol){
            continue;
        }
        
        if (!dictExists(db)) {
            //initialize again?
            return;
        }
        
        // Get  words from database that start with same letter
        std::vector<std::string> letterVector = getWordsStartingWith(db, word[0], word.length());
        if(letterVector.empty()){
            return;
        }
        // Process each candidate word from database
        for (const auto& dictWord : letterVector) {
            if (word.size() != dictWord.size()) {
                continue; // Only consider words of same length
            }
            
            float current = 0;
            bool validWord = true;
            for (int charPos = 0; charPos < word.size(); charPos++) {
                current += distBFS(word[charPos], dictWord[charPos], keyGraph);
            }
            current += weight;
            //printf("Past BFS search\n");
            insertClosestMatch(closestResponses, closestResponsesDist,dictWord,largestMinDist,current);
        }
    }
}
std::vector<std::string> autoSuggest(sqlite3* db, std::string word, std::vector<std::vector<char>> &keyGraph){
    //printf("new word\n");
    int stringIndex =0;
    while (stringIndex<word.size()){
        word[stringIndex] = std::tolower(word[stringIndex]);
        stringIndex++;
    }
    float weight = .1;
    std::vector<std::string> combinations;
    std::vector<std::string> combinationSave;
    std::vector<std::string> closestResponses;
    std::vector<int> closestResponsesDist;
    float largestMinDist = 1000;
    combinations.push_back(word);
    combinationSave = letterSwap(word);
    combinations.insert(combinations.end(), combinationSave.begin(), combinationSave.end());
    //printf("past swap\n");
    combinationSave.clear();
    combinationSave = letterInsert(word);
    combinations.insert(combinations.end(), combinationSave.begin(), combinationSave.end());
    combinationSave.clear();
    //printf("Before og word\n")
    combinationSave = letterDeletion(word);
    combinations.insert(combinations.end(), combinationSave.begin(), combinationSave.end());
    combinationSave.clear();
    //printf("Before word exists\n");
    if(wordExists(db, combinations, combinationSave)){
        //printf("in word exists\n")
        for(int i = 0; i< combinationSave.size(); i++){
            insertClosestMatch(closestResponses, closestResponsesDist,combinationSave[i],largestMinDist,weight);
        }
    }
    //printf("after word exists, before closest word search\n");
    closestWordSearch(combinations, closestResponses, closestResponsesDist, db, keyGraph, largestMinDist, weight);
    weight++;
    //printf("Word: %ls\n", word.c_str());
    printf("Best Responses (%s): ", word.c_str());
    for(int i=0;i<closestResponses.size();i++){
        printf("%s ",closestResponses[i].c_str());
    }
    printf("\n");
    return closestResponses;
}