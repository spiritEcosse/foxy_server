#pragma once
#include <string>
#include <unordered_map>

namespace api::v1 {
    struct TransparentHasher {
        using is_transparent = void;  // This makes the hasher transparent (required for heterogeneous lookup)

        size_t operator()(std::string_view key) const noexcept {
            return std::hash<std::string_view>{}(
                key);  // Use hash for string_view (works with both string and string_view)
        }

        size_t operator()(const std::string& key) const noexcept {
            return std::hash<std::string_view>{}(key);  // Call the same hash function for std::string
        }

        size_t operator()(const char* key) const noexcept {
            return std::hash<std::string_view>{}(key);  // Call the same hash function for C-style string
        }
    };

    using TransparentMap = decltype(std::unordered_map<std::string, std::string, TransparentHasher, std::equal_to<>>());
}
