#pragma once

#include <drogon/drogon.h>
#include <jwt-cpp/jwt.h>

namespace api::utils::jwt {
    class JWT {
    public:
        [[nodiscard]] static std::tuple<drogon::HttpStatusCode, Json::Value>
        verifyGoogleToken(const std::string& token);

    private:
        std::string token;
        std::int64_t expiration;
    };
}
