#include <string>
#include <vector>

std::vector<std::wstring> dictGraphSave;
std::vector<std::vector<std::wstring>> dictGraphCount;
bool binarySearch(std::vector<std::wstring>letterVector, int left, int right, std::wstring word);
int distBFS(std::wstring typedLetter, std::wstring dictionaryLetter);
std::vector<std::wstring> bruteForceSearch(std::wstring word);
