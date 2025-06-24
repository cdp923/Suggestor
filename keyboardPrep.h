#include <vector>
#include <string>

extern int keyNumber; 
extern std::vector<std::vector<char>> keyGraph;
extern std::vector<std::vector<std::wstring>> dictGraph;


std::vector<std::vector<char>> initKeyboard();
std::vector<std::vector<std::wstring>> initDictionary();
std::vector<std::wstring> autoComplete(std::wstring input);
int indexOfFirstChar(std::wstring input);
bool containSymbols(std::wstring input);
std::vector<std::wstring> resizeVector(std::vector<std::wstring> suggestions);