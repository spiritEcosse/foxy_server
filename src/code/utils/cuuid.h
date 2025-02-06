#pragma once
#include <uuid/uuid.h>
#include <string>

[[nodiscard]] inline std::string cuuid() {
    uuid_t uuid;
    char uuid_str[37];

    uuid_generate(uuid);

    uuid_unparse(uuid, uuid_str);

    return uuid_str;
}