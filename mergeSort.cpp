#include "mergeSort.h"
#include <string>
#include <iostream>

std::vector<std::wstring> Merge(std::vector<std::wstring>vectorPerLetter, int left,int middle, int right){
    int leftSize = middle - left + 1;
    int rightSize = right - middle;
    std::vector<std::wstring> leftVector(leftSize);
    std::vector<std::wstring> rightVector(rightSize);

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
std::vector<std::wstring> MergeSort(std::vector<std::wstring>wordVector, int left, int right){
        if (left<right){
            int middle = (left +right)/2;
            wordVector = MergeSort(wordVector, left, middle);
            wordVector = MergeSort(wordVector, middle+1, right);
            wordVector =  Merge(wordVector, left, middle, right);
        }
        return wordVector;
}
std::vector<std::vector<std::wstring>> CallMergeSort(std::vector<std::vector<std::wstring>>wordVector, int left, int right){
    int letterCount = 0;
    while(letterCount<wordVector.size()){
        wordVector[letterCount] = MergeSort(wordVector[letterCount], 0, wordVector[letterCount].size()-1);
        letterCount++;
    }
    return wordVector;
}
