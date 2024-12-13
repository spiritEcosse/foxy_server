#pragma once
#include <string>

inline void getenv(const char *name, std::string &env) {
    if(const char *ret = std::getenv(name)) {
        env = std::string(ret);
    } else {
        throw std::runtime_error(std::string("Environment variable not found: ") + name);
    }
}
