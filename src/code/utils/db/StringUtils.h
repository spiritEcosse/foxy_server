#pragma once

#include <string>
#include <random>
#include <fmt/format.h>
#include <fmt/ranges.h>

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

inline std::string removeHtmlTags(const std::string& input) {
    std::string result;
    bool insideTag = false;

    for(char c: input) {
        if(c == '<') {
            insideTag = true;
            continue;
        }
        if(c == '>') {
            insideTag = false;
            continue;
        }
        if(!insideTag) {
            result += c;
        }
    }

    return result;
}

inline std::string truncateText(const std::string_view& text, size_t maxLength = 280) {
    if(text.length() <= maxLength) {
        return std::string(text);
    }

    std::vector<std::string> words;
    words.reserve(text.length() / 5);

    size_t start = 0;
    size_t end = text.find(' ');
    while(end != std::string_view::npos) {
        words.emplace_back(text.substr(start, end - start));
        start = end + 1;
        end = text.find(' ', start);
    }
    if(start < text.length()) {
        words.emplace_back(text.substr(start));
    }

    std::string newText;
    newText.reserve(maxLength);

    while(!words.empty()) {
        newText = fmt::format("{}", fmt::join(words, " "));
        if(newText.length() <= maxLength) {
            break;
        }
        words.pop_back();
    }

    if(newText.empty()) {
        return std::string(text.substr(0, maxLength - 3)) + "...";
    }

    if(size_t lastSpacePos = newText.rfind(' ', maxLength - 3); lastSpacePos != std::string::npos) {
        newText = newText.substr(0, lastSpacePos) + "...";
    } else {
        newText = newText.substr(0, maxLength - 3) + "...";
    }

    return newText;
}
