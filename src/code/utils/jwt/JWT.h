#pragma once

#include "IClientImpl.h"

#include <drogon/drogon.h>

namespace api::utils::jwt {
    class JWT final : public v1::IClientImpl {
    public:
        [[nodiscard]] static std::tuple<drogon::HttpStatusCode, Json::Value> verifyGoogleToken(std::string token);
    };
}
