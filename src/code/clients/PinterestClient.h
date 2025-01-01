#pragma once

#include "IClient.h"
#include "env.h"

namespace api::v1 {
    class Pin;

    class PinterestClient final : public IClient<PinterestClient, Pin> {
        static constexpr std::string_view accessToken = PINTEREST_ACCESS_TOKEN;
        std::string auth() const override;
        bool setPostId(const cpr::Response& response, const Json::Value& jsonResponse, Pin* pin) const override;

    public:
        [[nodiscard]] bool uploadVideos(const Pin* pin) const;

        static constexpr std::string_view field_media_id = "media_id";
        static std::string apiUploadMedia;
        static std::string apiCreatePost;
        static constexpr std::string_view clientName = "Pinterest";
    };
}
