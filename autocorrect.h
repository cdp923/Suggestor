#include <string>
#include <vector>

//std::vector<std::wstring> dictGraphSave;
//std::vector<std::vector<std::wstring>> dictGraphCount;
bool binarySearch(std::vector<std::wstring>&letterVector, int left, int right, std::wstring& word);
int distBFS(std::wstring typedLetter, std::wstring dictionaryLetter,std::vector<std::vector<char>> keyGraph);
std::vector<std::wstring> simpleAutoCorrect(std::wstring word, std::vector<std::vector<std::wstring>>dictGraph,std::vector<std::vector<char>> keyGraph);
std::vector<std::wstring> letterSwapAutoCorrect(std::wstring word, std::vector<std::vector<std::wstring>>& dictGraph,std::vector<std::vector<char>>& keyGraph);
std::vector<std::wstring> fullAutoCorrect(std::wstring word, std::vector<std::vector<std::wstring>>&dictGraph,std::vector<std::vector<char>> &keyGraph);
int isWordCheck(std::wstring word, std::vector<std::wstring> &combinations, std::vector<std::wstring> &closestResponses);
void closestWordSearch(std::vector<std::wstring> &combinations, std::vector<std::wstring> &closestResponses,
    std::vector<int> &closestResponsesDist, std::vector<std::vector<std::wstring>>&dictGraph, std::vector<std::vector<char>>&keyGraph);
std::vector<std::wstring> letterSwap(std::wstring word);
std::vector<std::wstring> letterInsert(std::wstring word);
std::vector<std::wstring> letterDeletion(std::wstring word);