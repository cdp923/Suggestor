#include "keyboardAndDictionary.h"
#include "autocomplete.h"
#include "mergeSort.h" //runtime is to long. used.sort instead
#include "autocorrect.h"
#include <sstream>
#include <windows.h> 

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
    keyGraphInitilizer = {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z};
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
int indexOfChar(char input){
    return input-97;
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

int main(){
    std::vector<std::vector<std::wstring>> dictGraph = initDictionary();
    std::vector<std::vector<char>> keyGraph = initKeyboard();
    //std::vector<std::wstring> test= letterDeletion(L"mixac");
    //std::vector<std::wstring> test= letterInsert(L"mixac");
    //std::vector<std::wstring> test= letterSwap(L"mixac");
    /*
    std::vector<std::wstring> test= letterSwap(L"pizza");
    std::vector<std::vector<std::wstring>> smallDict = {
    { L"pizza", L"pizaz", L"piazz"},  // Only a few words for 'p'
    {L"xtra", L"xart", L"xrat"}      // Only a few words for 'x'
};
std::vector<std::wstring> result = letterSwapAutoCorrect(L"pizza", smallDict, keyGraph);
*/
    ///*
    std::string input;
    printf("Type: ");
    std::getline(std::cin, input);
    std::istringstream iss(input);
    std::string seperate;
    while ( getline( iss, seperate, ' ' ) ) {
        int bufferSize = MultiByteToWideChar(CP_UTF8, 0, seperate.c_str(), -1, nullptr, 0);
        std::wstring word(bufferSize - 1, L' ');
        MultiByteToWideChar(CP_UTF8, 0, seperate.c_str(), -1, &word[0], bufferSize);
        //std::vector<std::wstring> wordCorrectior = fullAutoCorrect(word, dictGraph, keyGraph);
        //std::vector<std::wstring> test= letterInsert(L"mixac");
        std::vector<std::wstring> wordCorrectior = letterSwapAutoCorrect(word, dictGraph, keyGraph);
        //std::vector<std::wstring> wordCorrectior = simpleAutoCorrect(word, dictGraph, keyGraph);
        //std::vector<std::wstring> test= letterSwap(L"systen");
    }
    //*/
/*
    //std::vector<std::wstring> wordCompletiot = autoComplete(L"wi", dictGraph);
    std::wstring word = L"acre";
    std::vector<std::wstring> wordCorrectior = autoCorrect(word, dictGraph, keyGraph);
    word = L"acrw";
    std::vector<std::wstring> wordCorrectioe = autoCorrect(word, dictGraph, keyGraph);
    word = L"acrq";
    std::vector<std::wstring> wordCorrectiow = autoCorrect(word, dictGraph, keyGraph);
    word = L"acry";
    std::vector<std::wstring> wordCorrectioq = autoCorrect(word, dictGraph, keyGraph);
*/
    return 0;
}
/*
cd ..
cd ..
cd projects\textEditorAutoCorrectComplete
g++ autocomplete.cpp keyboardAndDictionary.cpp mergeSort.cpp reverseInsertionSort.cpp autocorrect.cpp -o keyboardAndDictionary.exe
keyboardAndDictionary.exe
*/

