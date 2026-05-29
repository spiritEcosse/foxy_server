#pragma once
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace api::v1 {

    struct ConfigError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    inline std::string getEnv(const char *name, const char *defaultValue = nullptr) {
        if(const char *val = std::getenv(name); val)
            return val;
        if(defaultValue)
            return defaultValue;
        throw ConfigError(std::string("Required environment variable not set: ") + name);
    }

}  // namespace api::v1
