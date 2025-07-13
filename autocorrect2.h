#include <string>
#include <vector>

bool binarySearch(std::vector<std::wstring>letterVector, int left, int right, std::wstring word);
int distBFS(std::wstring typedLetter, std::wstring dictionaryLetter,std::vector<std::vector<char>> keyGraph);
std::vector<std::wstring> letterSwapAutoCorrect(std::wstring word, std::vector<std::vector<std::wstring>>dictGraph,std::vector<std::vector<char>> keyGraph);
std::vector<std::wstring> fullAutoCorrect(std::wstring word, std::vector<std::vector<std::wstring>>&dictGraph,std::vector<std::vector<char>> &keyGraph);
std::vector<std::wstring> letterSwap(std::wstring word);
std::vector<std::wstring> letterInsert(std::wstring word);
std::vector<std::wstring> letterDeletion(std::wstring word);