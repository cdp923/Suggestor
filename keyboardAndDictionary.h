#include <vector>
#include <string>

extern int keyNumber; 
extern std::vector<std::vector<char>> keyGraph;
extern std::vector<std::vector<std::wstring>> dictGraph;
extern std::vector<std::wstring> wordCompletion;


std::vector<std::vector<char>> initKeyboard();
std::vector<std::vector<std::wstring>> initDictionary();
