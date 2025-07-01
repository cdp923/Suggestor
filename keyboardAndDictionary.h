#include <vector>
#include <string>

#define MAXKEYCONNECTIONS 6

extern int keyNumber; 
extern std::vector<std::vector<char>> keyGraph;
extern std::vector<std::vector<std::wstring>> dictGraph;
extern std::vector<std::wstring> wordCompletion;


std::vector<std::vector<char>> initKeyboard();
std::vector<std::vector<std::wstring>> initDictionary();
int indexOfFirstChar(std::wstring input);
bool containSymbols(std::wstring input);
int indexOfChar(char input);
