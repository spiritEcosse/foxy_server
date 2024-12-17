#pragma once

#include "IClient.h"
#include "env.h"

#ifndef PINTEREST_ACCESS_TOKEN
#error PINTEREST_ACCESS_TOKEN must be defined at compile-time
#endif

#ifndef PINTEREST_API_CREATE_ENDPOINT
#error PINTEREST_API_CREATE_ENDPOINT must be defined at compile-time
#endif

namespace api::v1 {
    class Pin;

    class PinterestClient final : public IClient<PinterestClient, Pin> {
        static constexpr std::string_view accessToken = PINTEREST_ACCESS_TOKEN;
        std::string auth() override;
        bool setPostId(const cpr::Response& response, const Json::Value& jsonResponse, Pin* pin) const override;

    public:
        static constexpr std::string_view apiCreatePost = PINTEREST_API_CREATE_ENDPOINT;
        static constexpr std::string_view clientName = "Pinterest";
    };
}
