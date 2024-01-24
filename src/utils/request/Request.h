#pragma once

inline int getInt(const std::string &input, int defaultValue) {
    if(input.empty()) {
        return defaultValue;
    }
    try {
        return std::stoi(input);
    } catch(const std::invalid_argument &e) {
        LOG_ERROR << e.what() << "; input:" << input;
        return defaultValue;
    }
}

inline bool canBeInt(const std::string &input) {
    if(input.empty()) {
        return false;
    }
    try {
        std::stoi(input);
        return true;
    } catch(const std::invalid_argument &e) {
        return false;
    }
}
