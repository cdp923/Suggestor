#include "autocorrect.h"
#include "keyboardAndDictionary.h"
#include "reverseInsertionSort.h"
#include <queue>
#include <Windows.h>

#define MAXSUGGESTIONS 3
bool binarySearch(std::vector<std::wstring>letterVector,int left, int right, std::wstring word){
    if(letterVector.empty()||word.empty()){
        return false;
    }
    int stringIndex =0;
    int middle = left+(right-left)/2;
    std::wstring current = letterVector[middle];
    //printf("In BinarySearch. Word: %ls, Current: %ls",word.c_str(), letterVector[middle].c_str() );
    if (left<=right){
        if (current == word){
            //printf("\n\nTrue! Word: %ls, Current: %ls\n\n",word.c_str(), letterVector[middle].c_str() );
            return true;
        }
        if (current > word ){
            return binarySearch(letterVector,left ,middle-1, word);
        }    
        return binarySearch(letterVector,middle+1 ,right, word);
        
    }
    //printf("returning false");
    return false;
}
int distBFS(char typedLetter, char dictionaryLetter, std::vector<std::vector<char>> keyGraph){
    if (typedLetter == '\0' || dictionaryLetter == '\0') { //remove this and add correct logic
        printf("Error: Invalid input characters\n");
        return -1;
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
    if (typedLetter == dictionaryLetter){
        return 0;
    }
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
                if(dictionaryLetter == keyGraph[current][i]){
                    //printf("\nDistance is %i\n", distance[currentConnection]);
                    return distance[currentConnection];
                }
                queue.push(keyGraph[current][i]);
            }
        }
        //printf("\n");
    }
    //printf("Distance out of loop is %i\n", distance[position]);
    return distance[position];
}
//implement character swaps and varying sizes
std::vector<std::wstring> simpleAutoCorrect(std::wstring word, std::vector<std::vector<std::wstring>>dictGraph, std::vector<std::vector<char>> keyGraph){
    int stringIndex =0;
    while (stringIndex<word.size()){
        word[stringIndex] = std::tolower(word[stringIndex]);
        stringIndex++;
    }
    std::vector<std::wstring> closestResponses;
    std::vector<int> closestResponsesDist;
    int index = indexOfFirstChar(word);
    std::vector<std::wstring>letterVector = dictGraph[index];
    printf("Word: %ls\n",word.c_str());
    if(binarySearch(letterVector, 0, letterVector.size(), word)==true||word.size()<2){
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
        //printf("Typed word: %ls, checking agaisnt: %ls \n", word.c_str(),dictGraph[index][i].c_str());
        int minLength = 0;
        int difference = 0;
        if(word.size()>dictGraph[index][i].size()){
            minLength = dictGraph[index][i].size();
            difference = word.size()-dictGraph[index][i].size();
        }else{
            minLength = word.size();
            difference = dictGraph[index][i].size() - word.size();
        }
        for (int x= 0; x<minLength;x++){
            current+=distBFS(word[x], dictGraph[index][i][x], keyGraph);
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
                closestResponses.insert(closestResponses.begin() + insertionPoint, dictGraph[index][i]);
                closestResponsesDist.insert(closestResponsesDist.begin() + insertionPoint, current);
                largestMinDist = closestResponsesDist[insertionPoint];
            }else if(current < closestResponsesDist[MAXSUGGESTIONS - 1]){
                int insertionPoint = 0;
                while (insertionPoint < MAXSUGGESTIONS && 
                    closestResponsesDist[insertionPoint] <= current) {
                    insertionPoint++;
                }
                closestResponses.insert(closestResponses.begin() + insertionPoint, dictGraph[index][i]);
                closestResponsesDist.insert(closestResponsesDist.begin() + insertionPoint, current);
                largestMinDist = closestResponsesDist[insertionPoint];
                
                closestResponses.pop_back();
                closestResponsesDist.pop_back();
            }
            //printf("Word Done\n");
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
    //printf("Word: %ls\n", word.c_str());
    printf("Best Responses: ");
    for(int i=0;i<closestResponses.size();i++){
        printf("%ls, ",closestResponses[i].c_str());
    }
    printf("\n");
    return closestResponses;
}
std::vector<std::wstring> letterSwap(std::wstring word){

    std::vector<std::wstring> combinations;
    //combinations.push_back(word); //word already being checked
    wchar_t save;
    for(int i=1; i<word.length()-1;i++){//skips first letter. Will come back to
        std::wstring newWord = word;
        save = newWord[i];
        newWord[i] = newWord[i+1];
        newWord[i+1] = save;
        combinations.push_back(newWord);
    }
    /*
    for(int i=0; i<combinations.size();i++){
        printf("%ls\n", combinations[i].c_str());
    }
    printf("%ls\n", word.c_str());
    */
    return combinations;
}
std::vector<std::wstring> letterInsert(std::wstring word){
    std::vector<std::wstring> combinations;
    //combinations.push_back(word); //word already being checked
    for(int index=1; index<word.length()+1;index++){ //skips first letter. Will implement in a seperate part
        wchar_t letter = 'a';
        while(letter!='z'+1){
            std::wstring convert;
            convert.push_back(letter);
            std::wstring newWord = word;
            newWord.insert(index, convert);
            combinations.push_back(newWord);
            letter++;
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
std::vector<std::wstring> letterSwapAutoCorrect(std::wstring word, std::vector<std::vector<std::wstring>>dictGraph, std::vector<std::vector<char>> keyGraph){
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
        std::vector<std::wstring>letterVector = dictGraph[keyIndex];
        printf("Word: %ls\n",combinations[index].c_str());
        if(binarySearch(letterVector, 0, letterVector.size(), combinations[index])==true||combinations[index].size()<2){
            closestResponses.push_back(combinations[index]);
            for(int i=0;i<closestResponses.size();i++){
                printf("Word match: %ls\n",closestResponses[i].c_str());
            }
            return closestResponses;
        }
        std::wstring combo = combinations[index];
        int responsesSaved = 0;
        int largestMinDist = 1000;
        int current;
        for (int i= 0; i<dictGraph[index].size();i++){
            current = 0;
            //printf("Typed word: %ls, checking agaisnt: %ls \n", word.c_str(),dictGraph[index][i].c_str());
            int minLength = 0;
            int difference = 0;
            if(combo.size()>dictGraph[index][i].size()){
                minLength = dictGraph[index][i].size();
                difference = combo.size()-dictGraph[index][i].size();
            }else{
                minLength = combo.size();
                difference = dictGraph[index][i].size() - combo.size();
            }
            for (int x= 0; x<minLength;x++){
                current+=distBFS(combo[x], dictGraph[index][i][x], keyGraph);
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
                    closestResponses.insert(closestResponses.begin() + insertionPoint, dictGraph[index][i]);
                    closestResponsesDist.insert(closestResponsesDist.begin() + insertionPoint, current);
                    largestMinDist = closestResponsesDist[insertionPoint];
                }else if(current < closestResponsesDist[MAXSUGGESTIONS - 1]){
                    int insertionPoint = 0;
                    while (insertionPoint < MAXSUGGESTIONS && 
                        closestResponsesDist[insertionPoint] <= current) {
                        insertionPoint++;
                    }
                    closestResponses.insert(closestResponses.begin() + insertionPoint, dictGraph[index][i]);
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
std::vector<std::wstring> fullAutoCorrect(std::wstring word, std::vector<std::vector<std::wstring>>dictGraph, std::vector<std::vector<char>> keyGraph){
    int stringIndex =0;
    while (stringIndex<word.size()){
        word[stringIndex] = std::tolower(word[stringIndex]);
        stringIndex++;
    }
    std::vector<std::wstring> combinations;
    std::vector<std::wstring> combinationsSave;
    std::vector<std::wstring> closestResponses;
    std::vector<int> closestResponsesDist;
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
    for(int index=0;index<combinations.size(); index++){
        int keyIndex = indexOfFirstChar(combinations[index]);
        std::vector<std::wstring>letterVector = dictGraph[keyIndex];
        printf("Word: %ls\n",combinations[index].c_str());
        if(binarySearch(letterVector, 0, letterVector.size(), combinations[index])==true||combinations[index].size()<2){
            closestResponses.push_back(combinations[index]);
            for(int i=0;i<closestResponses.size();i++){
                printf("Word match: %ls\n",closestResponses[i].c_str());
            }
        }
        std::wstring combo = combinations[index];
        int responsesSaved = 0;
        int largestMinDist = 1000;
        int current;
        for (int i= 0; i<dictGraph[keyIndex].size();i++){ //or dictGraph[index].size()
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