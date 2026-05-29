#pragma once
#include <string>
#include <unordered_map>

namespace api::v1 {
    struct TransparentStringHash {
        using is_transparent = void;

        size_t operator()(std::string_view key) const noexcept {
            return std::hash<std::string_view>{}(key);
        }

        size_t operator()(const std::string& key) const noexcept {
            return std::hash<std::string_view>{}(key);
        }

        size_t operator()(const char* key) const noexcept {
            return std::hash<std::string_view>{}(key);
        }
    };

    using TransparentMap =
        decltype(std::unordered_map<std::string, std::string, TransparentStringHash, std::equal_to<>>());
}
