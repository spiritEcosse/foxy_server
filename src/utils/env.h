#pragma once
#include <string>

inline bool getenv(const char *name, std::string &env) {
    const char *ret = getenv(name);
    if(ret)
        env = std::string(ret);
    return ret != nullptr;
}
