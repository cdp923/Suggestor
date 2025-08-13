#include "wordCombos.h"

std::vector<std::string> letterSwap(std::string word){
    std::vector<std::string> combinations;
    for(int i=0; i<word.length()-1;i++){//skips first letter. Will implement in a seperate part
        std::string newWord = word;
        char save = newWord[i];
        newWord[i] = newWord[i+1];
        newWord[i+1] = save;
        combinations.push_back(newWord);
    }
    
    return combinations;
}
std::vector<std::string> letterInsert(std::string word){
    std::vector<std::string> combinations;
    int maxCombinations = 500;
    //combinations.push_back(word); //word already being checked
    for(int index=1; index<word.length()+1;index++){ //skips first letter. Will implement in a seperate part
        for (char letter = 'a'; letter <= 'z'; letter++) {
            if(combinations.size()>maxCombinations){
                printf("Returning");
                return combinations;
            }
            std::string newWord = word;
            newWord.insert(index, 1, letter);
            combinations.push_back(newWord);
        }
    }
    return combinations;
}
std::vector<std::string> letterDeletion(std::string word){
    std::vector<std::string> combinations;
    for(int index=0; index<word.length();index++){
        std::string newWord = word;
        newWord.erase(newWord.begin()+index);
        if(newWord.size()<2){
            continue;
        }
        combinations.push_back(newWord);
    }
    return combinations;
}