#pragma once

#include "clients/IClient.h"
#include "utils/config.h"

namespace api::v1 {
    class Pin;

    class PinterestClient final : public IClient<PinterestClient, Pin> {
        static inline const std::string clientId = getEnv("PINTEREST_CLIENT_ID");
        static inline const std::string clientSecret = getEnv("PINTEREST_CLIENT_SECRET");

        std::string auth() const override;
        bool setPostId(const cpr::Response& response, const Json::Value& jsonResponse, Pin* pin) const override;
        bool setAccessToken() override;
        std::string getAccessToken() const override;

    public:
        explicit PinterestClient() : IClient() {
            setAccessToken();
        }

        [[nodiscard]] bool uploadVideos(const Pin* pin) const;

        static constexpr std::string_view field_media_id = "media_id";
        static std::string apiUploadMedia;
        static std::string apiCreatePost;
        static std::string tokenUrl;
        static constexpr std::string_view clientName = "Pinterest";
    };
}
