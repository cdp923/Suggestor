#include <vector>
#include <string>

extern int keyNumber; 
extern std::vector<std::vector<char>> keyGraph;
extern std::vector<std::vector<std::wstring>> dictGraph;


std::vector<std::vector<char>> initKeyboard();
std::vector<std::vector<std::wstring>> initDictionary();
std::vector<std::vector<std::wstring>> quickSort(std::vector<std::vector<std::wstring>> unsortedDictGraph);