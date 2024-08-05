#pragma once

#include <drogon/drogon.h>

namespace api::utils::jwt {
    class JWT {
    public:
        [[nodiscard]] static std::tuple<drogon::HttpStatusCode, Json::Value>
        verifyGoogleToken(const std::string& token);
    };
}
