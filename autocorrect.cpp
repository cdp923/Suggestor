#include "autocorrect.h"
#include "keyboardAndDictionary.h"
#include "reverseInsertionSort.h"
#include <queue>
#include <Windows.h>

#define MAXSUGGESTIONS 3
bool binarySearch(std::vector<std::wstring>&letterVector,int left, int right, std::wstring &word){
    if(letterVector.empty()||word.empty()){
        return false;
    }
    int stringIndex =0;
    int middle = left+(right-left)/2;
    if (left > right) {
        //printf("left > right\n");
        return false;
    }
    std::wstring current = letterVector[middle];
    //printf("In BinarySearch. Word: %ls, Current: %ls\n",word.c_str(), letterVector[middle].c_str() );
    //printf("letterVector.size(): %i\n", letterVector.size());
    //printf("left: %i\n", middle+1);
    //printf("right: %i\n", right);
        if (current == word){
            return true;
        }
        if (current > word ){
            //printf("current > word\n");
            return binarySearch(letterVector,left ,middle-1, word);
        }
        //printf("current < word\n");
        return binarySearch(letterVector,middle+1 ,right, word);
    //printf("returning false\n");
    return false;
}
int distBFS(char typedLetter, char dictionaryLetter, std::vector<std::vector<char>> keyGraph){
    //printf("In BFS search\n");
    if (typedLetter == '\0' || dictionaryLetter == '\0') { //remove this and add correct logic
        printf("Error: Invalid input characters\n");
        return -1;
    }
    if (typedLetter == dictionaryLetter){
        return 0;
    }
    std::vector<int> visited(keyGraph.size(), -1);
    std::vector<int> distance(keyGraph.size(), -1);
    std::queue<char> queue;
    char save;
    std::vector<char> stringVector;
    stringVector.push_back(typedLetter);
    int position = indexOfChar(typedLetter);
    int current = 0;
    distance[position] =0;
    visited[position] = 0;
    queue.push(typedLetter);
    //printf("Typed char: %c, checking agaisnt: %c \n", typedLetter,dictionaryLetter);
    while (!queue.empty()){
        save = queue.front();
        queue.pop();
        current = indexOfChar(save);
        if (current < 0 || current >= keyGraph.size()) {
            printf("Error: current_index %d out of bounds\n", current);
            continue;
        }
        //printf("keyGraph[%c] =", save);
        for (int i =0; i<keyGraph[current].size();i++){ //might need to pass keyGraph
            int currentConnection = indexOfChar(keyGraph[current][i]);
            //printf(" %c,", keyGraph[current][i]);
            if (currentConnection < 0 || currentConnection >= keyGraph.size()) {
                printf("(invalid neighbor index %d)", currentConnection);
                continue;
            }
            if (visited[currentConnection] == -1){
                visited[currentConnection]=0;
                distance[currentConnection] = distance[current]+1; //possibly break if distance[currentConnection] >2
                //if(distance[currentConnection]>2){
                //    printf("BFS continue\n");
                //    continue;
                //}
                if(dictionaryLetter == keyGraph[current][i]){
                    //printf("BFS return in loop\n");
                    //printf("\nDistance is %i\n", distance[currentConnection]);
                    return distance[currentConnection];
                }
                queue.push(keyGraph[current][i]);
            }
        }
        //printf("\n");
    }
    //printf("BFS return out of loop\n");
    //printf("Distance out of loop is %i\n", distance[position]);
    return distance[position]; //should always be 0, change later
}

std::vector<std::wstring> letterSwap(std::wstring word){

    std::vector<std::wstring> combinations;
    //combinations.push_back(word); //word already being checked
    for(int i=1; i<word.length()-1;i++){//skips first letter. Will come back to
        std::wstring newWord = word;
        wchar_t save = newWord[i];
        newWord[i] = newWord[i+1];
        newWord[i+1] = save;
        combinations.push_back(newWord);
    }
    
    
    return combinations;
}
std::vector<std::wstring> letterInsert(std::wstring word){
    std::vector<std::wstring> combinations;
    //combinations.push_back(word); //word already being checked
    for(int index=1; index<word.length()+1;index++){ //skips first letter. Will implement in a seperate part
        for (wchar_t letter = L'b'; letter <= L'z'; letter++) {
            std::wstring newWord = word;
            newWord.insert(index, 1, letter);
            combinations.push_back(newWord);
        }
    }
    /*
    printf("Combinations: ");
    for(int index=0; index<combinations.size();index++){
        printf("%ls, ", combinations[index].c_str());
    }
    */
    return combinations;
}
std::vector<std::wstring> letterDeletion(std::wstring word){
    std::vector<std::wstring> combinations;
    for(int index=1; index<word.length();index++){
        std::wstring newWord = word;
        newWord.erase(newWord.begin()+index);
        combinations.push_back(newWord);
    }
    /*
    printf("Combinations: ");
    for(int index=0; index<combinations.size();index++){
        printf("%ls, ", combinations[index].c_str());
    }
    */
    return combinations;
}
std::vector<std::wstring> letterSwapAutoCorrect(std::wstring word, std::vector<std::vector<std::wstring>>& dictGraph, std::vector<std::vector<char>>& keyGraph){
    int stringIndex =0;
    while (stringIndex<word.size()){
        word[stringIndex] = std::tolower(word[stringIndex]);
        stringIndex++;
    }
    std::vector<std::wstring> combinations;
    std::vector<std::wstring> closestResponses;
    std::vector<int> closestResponsesDist;
    combinations = letterSwap(word);
    for(int index=0;index<combinations.size(); index++){
        int keyIndex = indexOfFirstChar(word);
        if (keyIndex < 0 || keyIndex >= dictGraph.size()) {
            printf("Error: Invalid dictionary index\n");
            return closestResponses;
        }
        std::vector<std::wstring>letterVector = dictGraph[keyIndex];
        //printf("Word: %ls\n",combinations[index].c_str());
        if(binarySearch(letterVector, 0, letterVector.size()-1, combinations[index])==true||combinations[index].size()<2){
            closestResponses.push_back(combinations[index]);
            for(int i=0;i<closestResponses.size();i++){
                printf("Word match: %ls\n",closestResponses[i].c_str());
            }
            return closestResponses;
        }
        //printf("binarySearch = false\n");
        std::wstring combo = combinations[index];
        int responsesSaved = 0;
        int largestMinDist = 1000;
        int current;
        for (int i= 0; i<dictGraph[keyIndex].size();i++){
            current = 0;
            //printf("Typed word: %ls, checking agaisnt: %ls \n", word.c_str(),dictGraph[index][i].c_str());
            int minLength = 0;
            int difference = 0;
            if(combo.size()>dictGraph[keyIndex][i].size()){
                minLength = dictGraph[keyIndex][i].size();
                difference = combo.size()-dictGraph[keyIndex][i].size();
            }else{
                minLength = combo.size();
                difference = dictGraph[keyIndex][i].size() - combo.size();
            }
            for (int x= 0; x<minLength;x++){
                current+=distBFS(combo[x], dictGraph[keyIndex][i][x], keyGraph);
            }
            current += (difference*2);
            //printf("Past BFS search\n");
            if(largestMinDist > current){
                if(closestResponses.size()<MAXSUGGESTIONS){
                    int insertionPoint = 0;
                    while (insertionPoint < closestResponses.size() && 
                        closestResponsesDist[insertionPoint] <= current) {
                        insertionPoint++;
                    }
                    closestResponses.insert(closestResponses.begin() + insertionPoint, dictGraph[keyIndex][i]);
                    closestResponsesDist.insert(closestResponsesDist.begin() + insertionPoint, current);
                    largestMinDist = closestResponsesDist[insertionPoint];
                }else if(current < closestResponsesDist[MAXSUGGESTIONS - 1]){
                    int insertionPoint = 0;
                    while (insertionPoint < MAXSUGGESTIONS && 
                        closestResponsesDist[insertionPoint] <= current) {
                        insertionPoint++;
                    }
                    closestResponses.insert(closestResponses.begin() + insertionPoint, dictGraph[keyIndex][i]);
                    closestResponsesDist.insert(closestResponsesDist.begin() + insertionPoint, current);
                    largestMinDist = closestResponsesDist[insertionPoint];
                    
                    closestResponses.pop_back();
                    closestResponsesDist.pop_back();
                }
                //printf("Word Done\n");
            }

        }
    }
    //printf("Word: %ls\n", word.c_str());
    printf("Best Responses: ");
    for(int i=0;i<closestResponses.size();i++){
        printf("%ls, ",closestResponses[i].c_str());
    }
    printf("\n");
    return closestResponses;
}
void closestWordSearch(std::vector<std::wstring> &combinations, std::vector<std::wstring> &closestResponses,
    std::vector<int> &closestResponsesDist, std::vector<std::vector<std::wstring>>&dictGraph, std::vector<std::vector<char>>&keyGraph){ 
    int largestMinDist = 1000;
    for(int index=0;index<combinations.size(); index++){
        //int largestMinDist = 1000;
        int keyIndex = indexOfFirstChar(combinations[index]);
        //printf("%i", dictGraph[keyIndex].size());
        std::wstring combo = combinations[index];
        int responsesSaved = 0;
        int current;
        for (int i= 0; i<dictGraph[keyIndex].size();i++){ //or dictGraph[index].size()
            if (combo.empty() || dictGraph[keyIndex][i].empty()) continue;
            current = 0;
            //printf("Word: %ls, checking agaisnt: %ls \n", combo.c_str(),dictGraph[keyIndex][i].c_str());
            int minLength = 0;
            int difference = 0; //Gives weight to additional or missing letters
            /*
            if(combo.size()>dictGraph[keyIndex][i].size()){
                minLength = dictGraph[keyIndex][i].size();
                difference = combo.size()-dictGraph[keyIndex][i].size();
            }else{
                minLength = combo.size();
                difference = dictGraph[keyIndex][i].size() - combo.size();
            }
            for (int x= 0; x<minLength;x++){
                current+=distBFS(combo[x], dictGraph[keyIndex][i][x], keyGraph);
            }
            current += (difference*2);
            */
            if(combo.size()!=dictGraph[keyIndex][i].size()){
                //printf("continue\n");
                continue;
            }
            for (int x= 0; x<combo.size();x++){
                current+=distBFS(combo[x], dictGraph[keyIndex][i][x], keyGraph);
            }
            //printf("Past BFS search\n");
            if(largestMinDist > current){
                printf("New smallest dist: (%ls, %ls, %i) \n", dictGraph[keyIndex][i].c_str(), combo.c_str(), current);
                //insert in order to reduce time spent searching
                if(closestResponses.size()<MAXSUGGESTIONS){
                    int insertionPoint = 0;
                    while (insertionPoint < closestResponses.size() && 
                        closestResponsesDist[insertionPoint] <= current) {
                        insertionPoint++;
                    }
                    closestResponses.insert(closestResponses.begin() + insertionPoint, dictGraph[keyIndex][i]);
                    closestResponsesDist.insert(closestResponsesDist.begin() + insertionPoint, current);
                    largestMinDist = closestResponsesDist[insertionPoint];
                }else if(current < closestResponsesDist[MAXSUGGESTIONS - 1]){
                    int insertionPoint = 0;
                    while (insertionPoint < MAXSUGGESTIONS && 
                        closestResponsesDist[insertionPoint] <= current) {
                        insertionPoint++;
                    }
                    closestResponses.insert(closestResponses.begin() + insertionPoint, dictGraph[keyIndex][i]);
                    closestResponsesDist.insert(closestResponsesDist.begin() + insertionPoint, current);
                    largestMinDist = closestResponsesDist[insertionPoint];
                    
                    closestResponses.pop_back();
                    closestResponsesDist.pop_back();
                }
                //printf("Word Done\n");
            }

        }
    }
}
int isWordCheck(std::wstring word, std::vector<std::wstring> &combinations, std::vector<std::wstring> &closestResponses){
    for(int index=0;index<combinations.size(); index++){
        int keyIndex = indexOfFirstChar(combinations[index]);
        std::vector<std::wstring>letterVector = dictGraph[keyIndex];
        printf("Word: %ls\n",combinations[index].c_str());
        if(binarySearch(letterVector, 0, letterVector.size()-1, combinations[index])==true||combinations[index].size()<2){
            closestResponses.push_back(combinations[index]);
            printf("Word exists: %ls (%ls)\n", combinations[index].c_str(), word.c_str());
            return 0;
        }
    }
    return 1;
}
std::vector<std::wstring> fullAutoCorrect(std::wstring word, std::vector<std::vector<std::wstring>>&dictGraph, std::vector<std::vector<char>> &keyGraph){
    int stringIndex =0;
    while (stringIndex<word.size()){
        word[stringIndex] = std::tolower(word[stringIndex]);
        stringIndex++;
    }
    std::vector<std::wstring> combinations;
    std::vector<std::wstring> combinationsSave;
    std::vector<std::wstring> closestResponses;
    std::vector<int> closestResponsesDist;
    combinations.push_back(word);
    combinationsSave = letterSwap(word);
    for(int index = 0; index<combinationsSave.size();index++){
        combinations.push_back(combinationsSave[index]);
    }
    combinationsSave = letterInsert(word);
    for(int index = 0; index<combinationsSave.size();index++){
        combinations.push_back(combinationsSave[index]);
    }
    combinationsSave = letterDeletion(word);
    for(int index = 0; index<combinationsSave.size();index++){
        combinations.push_back(combinationsSave[index]);
    }
    if(isWordCheck(word, combinations, closestResponses)==0){
        return closestResponses;
    }
    printf("No words match %ls\n", combinations[0].c_str());
    closestWordSearch(combinations, closestResponses, closestResponsesDist, dictGraph, keyGraph);
    //printf("Word: %ls\n", word.c_str());
    printf("Best Responses: ");
    for(int i=0;i<closestResponses.size();i++){
        printf("%ls ",closestResponses[i].c_str());
    }
    printf("\n");
    return closestResponses;
}
