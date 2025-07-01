#include "autocorrect.h"
#include "keyboardAndDictionary.h"
#include "reverseInsertionSort.h"
#include <queue>

#define MAXSUGGESTIONS 3
bool binarySearch(std::vector<std::wstring>letterVector,int left, int right, std::wstring word){
    if(letterVector.empty()||word.empty()){
        printf("returning false");
        return false;
    }
    int stringIndex =0;
    int middle = left+(right-left)/2;
    std::wstring current = letterVector[middle];
    printf("In BinarySearch. Word: %ls, Current: %ls",word.c_str(), letterVector[middle].c_str() );
    if (left<=right){
        if (current == word){
            printf("\n\nTrue! Word: %ls, Current: %ls\n\n",word.c_str(), letterVector[middle].c_str() );
            return true;
        }
        if (current > word ){
            return binarySearch(letterVector,left ,middle-1, word);
        }    
        return binarySearch(letterVector,middle+1 ,right, word);
        
    }
    printf("returning false");
    return false;
}
int distBFS(char typedLetter, char dictionaryLetter){
    std::vector<int> visited(MAXKEYCONNECTIONS);
    std::vector<int> pathNumber(MAXKEYCONNECTIONS);
    std::vector<int> distance(MAXKEYCONNECTIONS);
    std::vector<std::vector<char>> distanceSave(MAXKEYCONNECTIONS);
    std::queue<char> queue;
    char save;
    for (int x = 0; x<keyGraph.size();x++){ //initialize values of visited, path number and distance lists
        visited[x]=-1;
        pathNumber[x]=0;
        distance[x]=-1;
    }
    std::vector<char> stringVector;
    stringVector.push_back(typedLetter);
    int position = indexOfChar(typedLetter);
    int current = 0;
    distance[position] =0;
    queue.push(typedLetter);
    while (!queue.empty()){
        save = queue.front();
        queue.pop();
        current = indexOfChar(save);
        for (int i =0; i<keyGraph[current].size();i++){ //might need to pass keyGraph
            int currentConnection = indexOfChar(keyGraph[current][i]);
            if (visited[currentConnection] == -1){
                visited[currentConnection]=0;
                pathNumber[currentConnection] = pathNumber[current]+1;
                distance[currentConnection] = distance[current]+1; //possibly break if distance[currentConnection] >2
                if(dictionaryLetter == keyGraph[current][i]){
                    return distance[currentConnection];
                }
                queue.push(keyGraph[current][i]);
            }
        }
    }
    return distance[position];
}
std::vector<std::wstring> autoCorrect(std::wstring word, std::vector<std::vector<std::wstring>>dictGraph){
    int stringIndex =0;
    while (stringIndex<word.size()){
        word[stringIndex] = std::tolower(word[stringIndex]);
        stringIndex++;
    }
    std::vector<std::wstring> closestResponses;
    std::vector<int> closestResponsesDist;
    int index = indexOfFirstChar(word);
    std::vector<std::wstring>letterVector = dictGraph[index];
    if(binarySearch(letterVector, 0, letterVector.size(), word)==true){
        closestResponses.push_back(word);
        for(int i=0;i<closestResponses.size();i++){
            printf("%ls\n",closestResponses[i].c_str());
        }
        return closestResponses;
    }

    int responsesSaved = 0;
    int largestMinDist = 1000;
    int current;
    for (int i= 0; i<dictGraph[index].size();i++){
        current = 0;
        for (int x= 0; x<word.size();x++){
            current+=distBFS(word[x], dictGraph[index][i][x]);
        }
        if(largestMinDist > current){
            if(closestResponses.size()<MAXSUGGESTIONS){
                int insertionPoint = 0;
                while (insertionPoint < closestResponses.size() && 
                    closestResponsesDist[insertionPoint] <= current) {
                    insertionPoint++;
                }
                closestResponses.insert(closestResponses.begin() + insertionPoint, dictGraph[index][i]);
                closestResponsesDist.insert(closestResponsesDist.begin() + insertionPoint, current);
            }else if(current < closestResponsesDist[MAXSUGGESTIONS - 1]){
                int insertionPoint = 0;
                while (insertionPoint < MAXSUGGESTIONS && 
                    closestResponsesDist[insertionPoint] <= current) {
                    insertionPoint++;
                }
                closestResponses.insert(closestResponses.begin() + insertionPoint, dictGraph[index][i]);
                closestResponsesDist.insert(closestResponsesDist.begin() + insertionPoint, current);
                
                closestResponses.pop_back();
                closestResponsesDist.pop_back();
            }
        }
            /* Dont have to sort if already sorted
            int tempDist;
            std::wstring tempWord;
            int leftIndex;
            for(int i = 0; i < closestResponsesDist.size(); i++){ //sort
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
        }
        */
        /*
        int temp;
        int left;
        for(int i=0;i<closestResponsesDist.size();i++){
            temp = closestResponsesDist[i];
            left = i-1;
            while(left>=0 &&closestResponsesDist[left]>temp){
                closestResponsesDist[left+1]=closestResponsesDist[left];
                closestResponses[left+1]=closestResponses[left];
                left--;
            }
            closestResponsesDist[left+1] = temp;
        }
        largestMinDist = closestResponsesDist[0];
        */
    }
    printf("Here");
    for(int i=0;i<closestResponses.size();i++){
        printf("%ls\n",closestResponses[i].c_str());
    }
    printf("\n");
    return closestResponses;
}