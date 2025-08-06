#include "algorithms.h"

#include <vector>
#include <string>
#include <queue>

int indexOfFirstChar(std::string input){
    return (int)input[0]-97;
}
int indexOfChar(char input){
    return input-97;
}
bool containSymbols(std::string input){
    int stringIndex =0;
    while (stringIndex<input.size()){
        if ((int)input[stringIndex]>122 || (int)input[stringIndex]<97){
            return true;

        }
        stringIndex++;
    }
    return false;
}
int isWordCheck(std::string word, std::vector<std::string> &combinations, std::vector<std::string> &closestResponses, std::vector<std::vector<std::string>>&dictGraph){
    for(int index=0;index<combinations.size(); index++){
        int keyIndex = indexOfFirstChar(combinations[index]);
        std::vector<std::string>letterVector = dictGraph[keyIndex];
        if(binarySearch(letterVector, 0, letterVector.size()-1, combinations[index])==true||combinations[index].size()<2){
            closestResponses.push_back(combinations[index]);
            printf("Word exists: %ls (%ls)\n", combinations[index].c_str(), word.c_str());
            return 0;
        }
    }
    return 1;
}
bool binarySearch(std::vector<std::string>&letterVector,int left, int right, std::string &word){
    if(letterVector.empty()||word.empty()){
        return false;
    }
    int stringIndex =0;
    int middle = left+(right-left)/2;
    if (left > right) {
        return false;
    }
    std::string current = letterVector[middle];
    if (current == word){
        return true;
    }
    if (current > word ){
        return binarySearch(letterVector,left ,middle-1, word);
    }
    return binarySearch(letterVector,middle+1 ,right, word);
    return false;
}
bfsInfo wordDistBFS(sqlite3* db, std::string& word,std::vector<std::vector<char>>& keyGraph){
    //printf("In BFS search\n");
    bfsInfo bfsInfo;
    int totalDistance = -1;
    for(int index = 0; index<word.size();index++){
        std::vector<int> visited(keyGraph.size(), -1);
        std::vector<int> distance(keyGraph.size(), -1);
        std::queue<char> queue;
        char save;
        std::vector<char> stringVector;
        stringVector.push_back(word[index]);
        int position = indexOfChar(word[index]);
        int current = 0;
        distance[position] =0;
        visited[position] = 0;
        queue.push(word[index]);
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
            for (int i =0; i<keyGraph[current].size();i++){ 
                std::string wordSave = word;
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
                    wordSave[i] = keyGraph[current][i];
                    if(wordExists(db, wordSave)){
                        //printf("BFS return in loop\n");
                        //printf("\nDistance is %i\n", distance[currentConnection]);
                        totalDistance += distance[currentConnection];
                        bfsInfo.word.push_back(wordSave);
                        bfsInfo.dist.push_back(totalDistance);
                        wordSave[i] = word[i];
                    }
                    queue.push(keyGraph[current][i]);
                }
            }
            //printf("\n");
        }
        //printf("BFS return out of loop\n");
        //printf("Distance out of loop is %i\n", distance[position]);
    }
    return bfsInfo; //should always be 0, change later
}
std::vector<std::string> reverseInsertionSort(std::vector<std::string> closestResponses, std::vector<int> closestResponsesDist){
    int largestMinDist = 1000;
    int tempDist;
    std::string tempWord;
    int leftIndex;
    for(int i = 0; i < closestResponsesDist.size(); i++){
        tempDist = closestResponsesDist[i];
        tempWord = closestResponses[i];
        leftIndex = i-1;
        while(leftIndex>=0 &&closestResponsesDist[leftIndex]<tempDist){
            closestResponsesDist[leftIndex+1]=closestResponsesDist[leftIndex];
            closestResponses[leftIndex+1]=closestResponses[leftIndex];
            leftIndex--;
        }
        closestResponsesDist[leftIndex + 1] = tempDist;
        closestResponses[leftIndex + 1] = tempWord;
    }
    largestMinDist = closestResponsesDist[0];
    return closestResponses;
}
std::vector<std::string> Merge(std::vector<std::string>vectorPerLetter, int left,int middle, int right){
    int leftSize = middle - left + 1;
    int rightSize = right - middle;
    std::vector<std::string> leftVector(leftSize);
    std::vector<std::string> rightVector(rightSize);

    for(int i = 0; i < leftSize; i++){
        leftVector[i] = vectorPerLetter[left + i];
    }
    for(int i = 0; i < rightSize; i++){
        rightVector[i] = vectorPerLetter[middle + i+1];
    }
    int leftCount = 0;
    int rightCount = 0;
    int mergedCount = left;
    while (leftCount < leftSize && rightCount < rightSize){
        if (leftVector[leftCount] <= rightVector[rightCount]){
            vectorPerLetter[mergedCount]=leftVector[leftCount];
            leftCount++;
        }else{
            vectorPerLetter[mergedCount]=rightVector[rightCount];
            rightCount++;
        }
        mergedCount++;
    }
    while(leftCount<leftSize){
        vectorPerLetter[mergedCount]=leftVector[leftCount];
        leftCount++;
        mergedCount++;
    }
    while(rightCount<rightSize){
        vectorPerLetter[mergedCount]=rightVector[rightCount];
        rightCount++;
        mergedCount++;
    }
    return vectorPerLetter;
}
std::vector<std::string> MergeSort(std::vector<std::string>wordVector, int left, int right){
        if (left<right){
            int middle = (left +right)/2;
            wordVector = MergeSort(wordVector, left, middle);
            wordVector = MergeSort(wordVector, middle+1, right);
            wordVector =  Merge(wordVector, left, middle, right);
        }
        return wordVector;
}
std::vector<std::vector<std::string>> CallMergeSort(std::vector<std::vector<std::string>>wordVector, int left, int right){
    int letterCount = 0;
    while(letterCount<wordVector.size()){
        wordVector[letterCount] = MergeSort(wordVector[letterCount], 0, wordVector[letterCount].size()-1);
        letterCount++;
    }
    return wordVector;
}