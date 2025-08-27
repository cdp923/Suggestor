#include <vector>
#include <string>

int indexOfFirstChar(std::string input);
int indexOfChar(char input);
float distBFS(char typedLetter, char dictionaryLetter, const std::vector<std::vector<char>>& keyGraph);
void insertClosestMatch(std::vector<std::string>& closestResponses, std::vector<float>& closestResponsesDist,
    const std::string& dictWord, float largestMinDist, float current); 
