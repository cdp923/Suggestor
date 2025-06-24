#include "keyboardPrep.h"
#include "mergeSort.h" //runtime is to long. used.sort instead

#include <cstdio>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <algorithm>

int keyNumber = 26; 
std::vector<std::vector<char>> keyGraph;
std::vector<std::vector<std::wstring>> dictGraph(keyNumber);
std::vector<std::vector<char>> initKeyboard(){
    std::vector<std::vector<char>> keyGraphInitilizer;
    std::vector<char> q = {'a','w'};
    std::vector<char> w = {'q','a','s','d','e'};
    std::vector<char> e = {'w','s','d','r'};
    std::vector<char> r = {'e','d','f','t'};
    std::vector<char> t = {'r','f','g','h','y'};
    std::vector<char> y = {'t','g','j','u'};
    std::vector<char> u = {'y','h','j','i'};
    std::vector<char> i = {'u','j','k','o'};
    std::vector<char> o = {'i','k','l','p'};
    std::vector<char> p = {'o','l'};
    std::vector<char> a = {'q','w','s','z'};
    std::vector<char> s = {'w','a','z','x','d','e'};
    std::vector<char> d = {'e','s','x','c','f','r'};
    std::vector<char> f = {'r','d','c','v','g','t'};
    std::vector<char> g = {'t','f','v','b','h','y'};
    std::vector<char> h = {'y','g','b','n','j','u'};
    std::vector<char> j = {'u','h','n','m','k','i'};
    std::vector<char> k = {'i','j','m','l','o'};
    std::vector<char> l = {'o','k','p'};
    std::vector<char> z = {'a','s','x'};
    std::vector<char> x = {'z','s','d','c'};
    std::vector<char> c = {'x','d','f','v'};
    std::vector<char> v = {'c','f','g','b'};
    std::vector<char> b = {'v','g','h','n'};
    std::vector<char> n = {'b','h','j','m'};
    std::vector<char> m = {'n','j','k'};
    keyGraphInitilizer = {q,w,e,r,t,y,u,i,o,p,a,s,d,f,g,h,j,k,l,z,x,c,v,b,n,m};
    return keyGraphInitilizer;
} 
std::vector<std::vector<std::wstring>> initDictionary(){
    std::ifstream file("Dictionary.txt");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file." << std::endl;
    }
    std::string word;
    int counter = 0;
    int max = 0; 
    char firstLetter = 'a';
    while (std::getline(file, word))
    {
        if(word.empty()){
            continue;
        }
        if (word.at(0) != firstLetter){
            counter++;
            firstLetter++;
        }
        int stringIndex = 0;
        while (stringIndex<word.size()){
            word[stringIndex] = std::tolower(word[stringIndex]);
            stringIndex++;
        }
        std::wstring wideWord(word.begin(), word.end());
        dictGraph[counter].push_back(wideWord);
        //printf("%ls, ", wideWord.c_str());
    }
    file.close();
    for(int i = 0; i<dictGraph.size(); i++){
        std::sort(dictGraph[i].begin(), dictGraph[i].end());
    }
    return dictGraph;
}
int indexOfFirstChar(std::wstring input){
    return (int)input[0]-97;
}
bool containSymbols(std::wstring input){
    int stringIndex =0;
    while (stringIndex<input.size()){
        if ((int)input[stringIndex]>122 || (int)input[stringIndex]<97){
            return true;

        }
        stringIndex++;
    }
    return false;
}

std::vector<std::wstring> autoComplete(std::wstring input){
    std::vector<std::wstring> suggestionVector(5);
    int stringIndex =0;
    while (stringIndex<input.size()){
        input[stringIndex] = std::tolower(input[stringIndex]);
        stringIndex++;
    }
    if (input.size() <= 1 || containSymbols(input)){
        printf("return empty vector. input.size() <= 1 || containSymbols(input)");
        suggestionVector.resize(0);
        return suggestionVector;
    }
    int index = indexOfFirstChar(input);
    int suggestionsAdded =0;
    for(int count=0; count<dictGraph[index].size();count++){
        //printf("%ls, ", dictGraph[index][count].c_str());
        if (input.size()<=dictGraph[index][count].size()){
            if (input == dictGraph[index][count].substr(0, input.size())){
                if (suggestionsAdded <suggestionVector.size()){
                    suggestionVector[suggestionsAdded] = dictGraph[index][count];
                    suggestionsAdded++;
                }else{
                    break;
                }
            }
        }
    }
    suggestionVector.resize(suggestionsAdded);
    int i =0;
    while (i<suggestionVector.size()){
        printf("%ls, ", suggestionVector[i].c_str());
        i++;
    }
    return suggestionVector;
}
int main(){
    dictGraph = initDictionary();
    autoComplete(L"y");
    return 0;
}
