#include "keyboard.h"

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
/*
cd ..
cd ..
cd projects\textEditorAutoCorrectComplete
g++ autocomplete.cpp keyboardAndDictionary.cpp mergeSort.cpp reverseInsertionSort.cpp autocorrect.cpp -o keyboardAndDictionary.exe
keyboardAndDictionary.exe
thsi si a teft of the systen
*/