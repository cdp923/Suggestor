#include "algorithms.h"
#include "database/attributes.h"

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
bfsInfo wordDistBFS(sqlite3* db, std::string& word,const std::vector<std::vector<char>>& keyGraph){
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
                if(distance[currentConnection]>3){
                        printf("BFS continue\n");
                        continue;
                    }
                if (visited[currentConnection] == -1){
                    visited[currentConnection]=0;
                    distance[currentConnection] = distance[current]+1; //possibly break if distance[currentConnection] >2
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