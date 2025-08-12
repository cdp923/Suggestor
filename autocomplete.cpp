#include "autocomplete.h"
#include "resources/algorithms.h"
#include "resources/database/attributes.h"
#include <algorithm>


std::vector<std::string> autoComplete(sqlite3* db, std::string input){
    std::vector<std::string> suggestionVector(3);
    int stringIndex =0;
    while (stringIndex<input.size()){
        input[stringIndex] = std::tolower(input[stringIndex]);
        stringIndex++;
    }
    if (containSymbols(input)){
        printf("containSymbols(input)");
        suggestionVector.resize(0);
        return suggestionVector;
    }
    suggestionVector = getSubset(db, input);
    return suggestionVector;
}
/*
DB attributes: word_text, frequency, last_used_timestamp (maybe), language_code, is_common_typo_for and source
*/