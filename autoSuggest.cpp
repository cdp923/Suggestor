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
        std::vector<std::string> letterVector = getWordsStartingWith(db, word[0]);
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
            if (largestMinDist > current || closestResponses.size() < MAXSUGGESTIONS) {
                if (closestResponses.size() < MAXSUGGESTIONS) {
                    int insertionPoint = 0;
                    while (insertionPoint < closestResponses.size() && 
                        closestResponsesDist[insertionPoint] <= current) {
                        insertionPoint++;
                    }
                    closestResponses.insert(closestResponses.begin() + insertionPoint, dictWord);
                    closestResponsesDist.insert(closestResponsesDist.begin() + insertionPoint, (int)current);
                    
                    if (closestResponses.size() > 0) {
                        largestMinDist = (float)closestResponsesDist[closestResponses.size() - 1];
                    }
                } else if (current < closestResponsesDist[MAXSUGGESTIONS - 1]) {
                    int insertionPoint = 0;
                    while (insertionPoint < MAXSUGGESTIONS && 
                        closestResponsesDist[insertionPoint] <= current) {
                        insertionPoint++;
                    }
                    closestResponses.insert(closestResponses.begin() + insertionPoint, dictWord);
                    closestResponsesDist.insert(closestResponsesDist.begin() + insertionPoint, (int)current);
                    
                    closestResponses.pop_back();
                    closestResponsesDist.pop_back();
                    
                    largestMinDist = (float)closestResponsesDist[MAXSUGGESTIONS - 1];
                }
            }
        }
    }
}
std::vector<std::string> autoSuggest(sqlite3* db, std::string word, std::vector<std::vector<char>> &keyGraph){
    int stringIndex =0;
    while (stringIndex<word.size()){
        word[stringIndex] = std::tolower(word[stringIndex]);
        stringIndex++;
    }
    float weight = .1;
    std::vector<std::string> combinations;
    std::vector<std::string> closestResponses;
    std::vector<int> closestResponsesDist;
    float largestMinDist = 1000;
    combinations.push_back(word);
    printf("Before og word\n");
    if(wordExists(db, combinations)){
        //printf("in word exists\n");
        return closestResponses;
    }
    //printf("after word exists, before closest word search\n");
    closestWordSearch(combinations, closestResponses, closestResponsesDist, db, keyGraph, largestMinDist, weight);
    weight++;

    printf("Before swap\n");
    combinations.clear();
    combinations = letterSwap(word);
    if(wordExists(db, combinations)){
        return closestResponses;
    }
    closestWordSearch(combinations, closestResponses, closestResponsesDist, db, keyGraph, largestMinDist, weight);
    weight++;
    printf("Before insert\n");
    combinations.clear();
    combinations = letterInsert(word);
    if(wordExists(db, combinations)){
        return closestResponses;
    }
    closestWordSearch(combinations, closestResponses, closestResponsesDist, db, keyGraph, largestMinDist, weight);
    weight++;

    combinations.clear();
    printf("Before delete\n");
    combinations = letterDeletion(word);
    if(wordExists(db, combinations)){
        return closestResponses;
    }
    closestWordSearch(combinations, closestResponses, closestResponsesDist, db, keyGraph, largestMinDist, weight);
    weight++;
    printf("No words match %s\n", word.c_str());
    //printf("Word: %ls\n", word.c_str());
    printf("Best Responses: ");
    for(int i=0;i<closestResponses.size();i++){
        printf("%s ",closestResponses[i].c_str());
    }
    printf("\n");
    return closestResponses;
}