#include "autoSuggest.h"
#include "resources/algorithms.h"
#include "resources/wordCombos.h"
#include "database/attributes.h"
#include <queue>
#include <Windows.h>

#define MAXSUGGESTIONS 3

void closestWordSearch(std::vector<std::string> &combinations, std::vector<std::string> &closestResponses,
    std::vector<int> &closestResponsesDist, sqlite3* &db, const std::vector<std::vector<char>>&keyGraph, float &largestMinDist, float weight){ 
    for(int index=0;index<combinations.size(); index++){
        int keyIndex = indexOfFirstChar(combinations[index]);
        std::string combo = combinations[index];
        int responsesSaved = 0;
        float current;
            if (combo.empty()) {
                continue;
            }
            if (!dictExists(db)) {
                return;
            }
            current = 0;
            if(combo.size()!=db[keyIndex][i].size()){
                //printf("continue\n");
                continue;
            }
        for (int x= 0; x<combo.size();x++){
            current+=distBFS(combo[x], db, keyGraph);
        }
        current+=weight;
        //printf("Past BFS search\n");
        if(largestMinDist > current){
            //printf("New smallest dist: (%ls, %ls, %d) \n", db[keyIndex][i].c_str(), combo.c_str(), current);
            //insert in order to reduce time spent searching
            if(closestResponses.size()<MAXSUGGESTIONS){
                int insertionPoint = 0;
                while (insertionPoint < closestResponses.size() && 
                    closestResponsesDist[insertionPoint] <= current) {
                    insertionPoint++;
                }
                closestResponses.insert(closestResponses.begin() + insertionPoint, db[keyIndex][i]);
                closestResponsesDist.insert(closestResponsesDist.begin() + insertionPoint, current);
                largestMinDist = (float) closestResponsesDist[insertionPoint];
            }else if(current < closestResponsesDist[MAXSUGGESTIONS - 1]){
                int insertionPoint = 0;
                while (insertionPoint < MAXSUGGESTIONS && 
                    closestResponsesDist[insertionPoint] <= current) {
                    insertionPoint++;
                }
                closestResponses.insert(closestResponses.begin() + insertionPoint, db[keyIndex][i]);
                closestResponsesDist.insert(closestResponsesDist.begin() + insertionPoint, current);
                largestMinDist = (float) closestResponsesDist[insertionPoint];
                
                closestResponses.pop_back();
                closestResponsesDist.pop_back();
            }
            //printf("Word Done\n");
        }

    }
}
std::vector<std::string> autoCorrect(std::string word, sqlite3* db, std::vector<std::vector<char>> &keyGraph){
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
    if(wordExists(db, combinations)){
        return closestResponses;
    }
    closestWordSearch(combinations, closestResponses, closestResponsesDist, db, keyGraph, largestMinDist, weight);
    weight++;

    combinations.clear();
    combinations = letterSwap(word);
    if(wordExists(db, combinations)){
        return closestResponses;
    }
    closestWordSearch(combinations, closestResponses, closestResponsesDist, db, keyGraph, largestMinDist, weight);
    weight++;

    combinations.clear();
    combinations = letterInsert(word);
    if(wordExists(db, combinations)){
        return closestResponses;
    }
    closestWordSearch(combinations, closestResponses, closestResponsesDist, db, keyGraph, largestMinDist, weight);
    weight++;

    combinations.clear();
    combinations = letterDeletion(word);
    if(wordExists(db, combinations)){
        return closestResponses;
    }
    closestWordSearch(combinations, closestResponses, closestResponsesDist, db, keyGraph, largestMinDist, weight);
    weight++;
    printf("No words match %ls\n", word.c_str());
    //printf("Word: %ls\n", word.c_str());
    printf("Best Responses: ");
    for(int i=0;i<closestResponses.size();i++){
        printf("%ls ",closestResponses[i].c_str());
    }
    printf("\n");
    return closestResponses;
}
