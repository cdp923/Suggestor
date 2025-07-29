#include <string>
#include <vector>

//std::vector<std::wstring> dictGraphSave;
//std::vector<std::vector<std::wstring>> dictGraphCount;
bool binarySearch(std::vector<std::wstring>&letterVector, int left, int right, std::wstring& word);
int distBFS(std::wstring typedLetter, std::wstring dictionaryLetter,std::vector<std::vector<char>> keyGraph);
int isWordCheck(std::wstring word, std::vector<std::wstring> &combinations, std::vector<std::wstring> &closestResponses);
void closestWordSearch(std::vector<std::wstring> &combinations, std::vector<std::wstring> &closestResponses,
    std::vector<int> &closestResponsesDist, const std::vector<std::vector<std::wstring>>&dictGraph, const std::vector<std::vector<char>>&keyGraph, float &largestMinDist, float weight);
std::vector<std::wstring> letterSwap(std::wstring word);
std::vector<std::wstring> letterInsert(std::wstring word);
std::vector<std::wstring> letterDeletion(std::wstring word);
std::vector<std::wstring> autoCorrect(std::wstring word, std::vector<std::vector<std::wstring>>&dictGraph,std::vector<std::vector<char>> &keyGraph);