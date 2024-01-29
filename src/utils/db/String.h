#pragma once
#include <string>

std::string addExtraQuotes(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (c == '\'') {
            result += "''";
        } else {
            result += c;
        }
    }
    return result;
}
