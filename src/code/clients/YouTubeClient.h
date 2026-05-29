#pragma once

#include "clients/IClient.h"
#include "utils/config.h"

namespace api::v1 {
    class YouTube;

    class YouTubeClient final : public IClient<YouTubeClient, YouTube> {
        static inline const std::string refreshToken = getEnv("YOUTUBE_REFRESH_TOKEN");
        static inline const std::string clientId = getEnv("YOUTUBE_CLIENT_ID");
        static inline const std::string clientSecret = getEnv("YOUTUBE_CLIENT_SECRET");
        static constexpr std::string_view tokenUrl = "https://oauth2.googleapis.com/token";
        std::string auth() const override;
        bool setPostId(const cpr::Response& response, const Json::Value& jsonResponse, YouTube* post) const override;
        bool setAccessToken() override;
        std::string getAccessToken() const override;

    public:
        explicit YouTubeClient() : IClient() {
            setAccessToken();
        }

        [[nodiscard]] bool uploadVideo(YouTube* post) const;

        static constexpr std::string_view field_media_id = "media_id";
        static constexpr std::string_view apiUploadMedia = "https://www.googleapis.com/upload/youtube/v3/videos";
        static constexpr std::string_view apiCreatePost = "";
        static constexpr std::string_view clientName = "YouTube";
    };
}
