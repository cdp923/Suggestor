#include <vector>
#include <string>
std::vector<std::wstring> reverseInsertionSort(std::vector<std::wstring> closestResponses, std::vector<int> closestResponsesDist){
    int largestMinDist = 1000;
    int tempDist;
    std::wstring tempWord;
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
