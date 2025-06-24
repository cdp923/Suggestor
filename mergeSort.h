#include <vector>
#include <string>

std::vector<std::wstring> Merge(std::vector<std::wstring>vectorPerLetter, int left,int middle, int right);
std::vector<std::wstring> MergeSort(std::vector<std::wstring>wordVector, int left, int right);
std::vector<std::vector<std::wstring>> CallMergeSort(std::vector<std::vector<std::wstring>>wordVector, int left, int right);