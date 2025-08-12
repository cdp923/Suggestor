#include <vector>
#include <string>

int indexOfFirstChar(std::string input);
int indexOfChar(char input);
int distBFS(char typedLetter, char dictionaryLetter, const std::vector<std::vector<char>>& keyGraph);
void insertClosestMatch(std::vector<std::string>& closestResponses, std::vector<int>& closestResponsesDist,
    const std::string& dictWord, int largestMinDist, float current); 
