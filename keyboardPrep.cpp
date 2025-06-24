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
        std::wstring wideWord(word.begin(), word.end());
        dictGraph[counter].push_back(wideWord);
    }
    for(int i = 0; i<dictGraph.size(); i++){
        std::sort(dictGraph[i].begin(), dictGraph[i].end());
    }
    //printf("%d",dictGraph.size());
    printf("%d\n",max);
    file.close();
    return dictGraph;
}

int main(){
    dictGraph = initDictionary();
    return 0;
}
