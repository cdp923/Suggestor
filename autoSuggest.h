#include "resources/database/sqlite/sqlite3.h"
#include <string>
#include <vector>

void closestWordSearch(std::vector<std::string> &combinations, std::vector<std::string> &closestResponses,
    std::vector<int> &closestResponsesDist, std::vector<std::vector<std::string>>&dictGraph, const std::vector<std::vector<char>>&keyGraph, float &largestMinDist, float weight);
std::vector<std::string> autoSuggest(sqlite3* db,std::string word,std::vector<std::vector<char>> &keyGraph);