
#include <vector>
#include <string>

int indexOfFirstChar(std::string input);
int indexOfChar(char input);
bool containSymbols(std::string input);
int isWordCheck(std::string word, std::vector<std::string> &combinations, std::vector<std::string> &closestResponses, std::vector<std::vector<std::string>>&dictGraph);
bool binarySearch(std::vector<std::string>&letterVector, int left, int right, std::string& word);
int distBFS(char typedLetter, char dictionaryLetter,std::vector<std::vector<char>> keyGraph);
std::vector<std::string> reverseInsertionSort(std::vector<std::string> closestResponses, std::vector<int> closestResponsesDist);
std::vector<std::string> MergeSort(std::vector<std::string>wordVector, int left, int right);
