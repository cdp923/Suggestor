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

int distBFS(char typedLetter, char dictionaryLetter, const std::vector<std::vector<char>>& keyGraph){
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