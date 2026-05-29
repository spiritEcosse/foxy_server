#pragma once

#include "clients/IClientImpl.h"

#include <drogon/drogon.h>
#include <expected>

namespace api::utils::jwt {
    class JWT final : public v1::IClientImpl {
    public:
        [[nodiscard]] static std::expected<Json::Value, Json::Value> verifyGoogleToken(std::string token);
    };
}
