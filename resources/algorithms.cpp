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
std::vector<bool> lowerCase(std::string& word){
    std::vector<bool> caseSave;
    int stringIndex =0;
    while (stringIndex<word.size()){
        char charSave = word[stringIndex];
        word[stringIndex] = std::tolower(word[stringIndex]);
        if(charSave != word[stringIndex]){
            caseSave.push_back(false);
        } else{
            caseSave.push_back(true);
        }
        stringIndex++;
    }
    return caseSave;
}

float distBFS(char typedLetter, char dictionaryLetter, const std::vector<std::vector<char>>& keyGraph){
    //printf("In BFS search\n");
    if (typedLetter == '\0' || dictionaryLetter == '\0') { //remove this and add correct logic
        printf("Error: Invalid input characters\n");
        return -1;
    }
    if (typedLetter == dictionaryLetter){
        return 0;
    }
    std::vector<int> visited(keyGraph.size(), -1);
    std::vector<float> distance(keyGraph.size(), -1);
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
                distance[currentConnection] = distance[current]+.25; //possibly break if distance[currentConnection] >2
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
void insertClosestMatch(std::vector<std::string>& closestResponses, std::vector<float>& closestResponsesDist,
                        const std::string& dictWord, float largestMinDist, float current) 
    {
    for (const std::string& item : closestResponses) {
        if (item == dictWord) {
            return;
        }
    }
    if (largestMinDist > current || closestResponses.size() < MAXSUGGESTIONS) {
        if (closestResponses.size() < MAXSUGGESTIONS) {
            int insertionPoint = 0;
            while (insertionPoint < closestResponses.size() && 
                closestResponsesDist[insertionPoint] <= current) {
                insertionPoint++;
            }
            closestResponses.insert(closestResponses.begin() + insertionPoint, dictWord);
            closestResponsesDist.insert(closestResponsesDist.begin() + insertionPoint, (int)current);
            
            if (closestResponses.size() > 0) { //was creating error, might be able to remove now
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