#include "database/attributes.h"
#include <vector>
#include <string>

struct bfsInfo {
    std::vector<std::string> word;
    std::vector<int> dist;
};

int indexOfFirstChar(std::string input);
int indexOfChar(char input);
bool containSymbols(std::string input);
bfsInfo wordDistBFS(sqlite3* db, std::string& word, const std::vector<std::vector<char>> &keyGraph);
std::vector<std::string> reverseInsertionSort(std::vector<std::string> closestResponses, std::vector<int> closestResponsesDist);
