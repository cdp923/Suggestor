#include "autoSuggest.h"
#include "resources/algorithms.h"
#include "resources/database/attributes.h"

#include <algorithm>
#include <iostream>
#include <queue>
void printMatrix(std::vector<std::vector<float>> dp, std::string word, std::string dictWord){
    printf("    ");
    for(int x = 0; x<word.size(); x++){

        printf("%c ", dictWord[x]);
    }
    printf("\n");

    for(int x = 0; x<dp.size(); x++){
        for(int y= 0; y<dp.size();y++){
            if(y==0 && x==0){
                printf("  ");
            }
            if (y==0 && x>0 && x<word.size()+1){
                printf("%c ", word[x-1]);
            }
            
            printf("%i ", dp[x][y]);
        }
        printf("\n");
    }
}
void closestWordSearch(std::string& word, std::vector<std::string>& closestResponses,
    std::vector<float>& closestResponsesDist, sqlite3* db, const std::vector<std::vector<char>>& keyGraph, 
    float& largestMinDist, float weight) {

    int stringIndex =0;
    if (!dictExists(db)) {
        //initialize again?
        return;
    }
    float frequency =0.2;
    // Get words of the same length from database that start with same letter
    std::vector<std::string> letterVector = getWordsStartingWith(db, word[0], word.length());
    //std::vector<std::string> letterVector = getSubset(db, word[0]);
    if(letterVector.empty()){
        return;
    }
    for (const std::string& dictWord : letterVector) {
        /*
        if (word.size() != dictWord.size()) {//can take this out bc levenstein, I think
            continue; // Only consider words of same length
        }
        */

        frequency = getWordFrequency(db, dictWord);
        //printf("%d\n", frequency);
        //std::cerr << "Word: "<<dictWord << ", Frequency: " << frequency <<   std::endl;
        int column = dictWord.length();
        int row = word.length();

        std::vector<std::vector<float>> wordComparison(column + 1, std::vector<float>(row + 1, 0));
        for (int count = 0; count <= column; count++) {
            wordComparison[count][0] = count*.25;
        }

        for (int count = 0; count <= row; count++) {
            wordComparison[0][count] = count*25;
        }

        for (int i = 1; i <= column; i++) {
            for (int j = 1; j <= row; j++) {
                if (dictWord[i - 1] == word[j - 1]) {
                    wordComparison[i][j] = wordComparison[i - 1][j - 1];
                }
                else {
                    wordComparison[i][j] = std::min(
                                // Insert
                                (wordComparison[i][j - 1]+(float).5),
                                std::min(
                                    // Remove
                                    (wordComparison[i - 1][j]+(float).5),
                                    // Replace
                                    (wordComparison[i - 1][j - 1]+ distBFS(word[j-1], dictWord[i-1], keyGraph))));
                }
            }
        }
        //distBFS(word[j-1], dictWord[i-1], keyGraph);
        //printMatrix(dp, word, dictWord);
        weight = wordComparison[column][row]+frequency;
        //std::cerr << "Word: "<<word << ", Compared to " << dictWord << ", Weight: "<<weight<<  std::endl;
        //printf("Past BFS search\n");
        insertClosestMatch(closestResponses, closestResponsesDist,dictWord,largestMinDist,weight);
    }
}
std::vector<std::string> autoSuggest(sqlite3* db, std::string word, std::vector<std::vector<char>> &keyGraph){
    if(word == "" || containSymbols(word)){
        return std::vector<std::string>();
    }

    std::vector<std::string> closestResponses;
    std::vector<float> closestResponsesDist;
    float largestMinDist = 1000;
    float weight = 0;

    std::vector<bool> casePosition = lowerCase(word); 
    int stringIndex =0;
    while (stringIndex<word.size()){
        word[stringIndex] = std::tolower(word[stringIndex]);
        stringIndex++;
    }
    if(wordExists(db, word)){
        insertClosestMatch(closestResponses, closestResponsesDist, word, largestMinDist, weight);
    }
    closestWordSearch(word, closestResponses, closestResponsesDist, db, keyGraph, largestMinDist, weight);
    printf("Best Responses (%s): ", word.c_str());
    for(int i=0;i<closestResponses.size();i++){
        printf("%s ",closestResponses[i].c_str());
    }
    printf("\n");
    return closestResponses;
}