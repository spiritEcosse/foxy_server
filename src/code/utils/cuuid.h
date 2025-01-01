#pragma once
#include <uuid/uuid.h>
#include <string>

[[nodiscard]] inline std::string cuuid() {
    uuid_t uuid;
    char uuid_str[37];  // 36 characters + 1 for null terminator

    // Generate a UUID
    uuid_generate(uuid);

    // Unparse the UUID into a null-terminated string
    uuid_unparse(uuid, uuid_str);

    return uuid_str;  // Automatically trims to 36 characters
}