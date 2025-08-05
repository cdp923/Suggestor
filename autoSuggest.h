#include "database/attributes.h"

#include <string>
#include <vector>

void closestWordSearch(std::vector<std::string> &combinations, std::vector<std::string> &closestResponses,
    std::vector<int> &closestResponsesDist, std::vector<std::vector<std::string>>&dictGraph, const std::vector<std::vector<char>>&keyGraph, float &largestMinDist, float weight);
std::vector<std::string> autoCorrect(std::string word, sqlite3* db,std::vector<std::vector<char>> &keyGraph);