#ifndef STRING_H
#define STRING_H
#include <string>

inline std::string addExtraQuotes(const std::string& str) {
    std::string result;
    for(char c: str) {
        if(c == '\'') {
            result += "''";
        } else {
            result += c;
        }
    }
    return result;
}
#endif  // STRING_H
