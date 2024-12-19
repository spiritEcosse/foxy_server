#pragma once

#include "IClient.h"
#include "env.h"
#include <fmt/format.h>

#ifndef PINTEREST_ACCESS_TOKEN
#error PINTEREST_ACCESS_TOKEN must be defined at compile-time
#endif

#ifndef PINTEREST_API_HOST
#error PINTEREST_API_HOST must be defined at compile-time
#endif

namespace api::v1 {
    class Pin;

    class PinterestClient final : public IClient<PinterestClient, Pin> {
        static constexpr std::string_view accessToken = PINTEREST_ACCESS_TOKEN;
        std::string auth() const override;
        bool setPostId(const cpr::Response& response, const Json::Value& jsonResponse, Pin* pin) const override;

    public:
        static constexpr std::string media_id = "media_id";
        [[nodiscard]] bool uploadVideos(const Pin* pin) const;
        static std::string apiUploadMedia;
        static std::string apiCreatePost;
        static constexpr std::string_view clientName = "Pinterest";
    };
}
