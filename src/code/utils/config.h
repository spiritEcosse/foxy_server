#pragma once
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace api::v1 {

inline std::string getEnv(const char *name, const char *defaultValue = nullptr) {
    const char *val = std::getenv(name);
    if(val) return val;
    if(defaultValue) return defaultValue;
    throw std::runtime_error(std::string("Required environment variable not set: ") + name);
}

} // namespace api::v1
