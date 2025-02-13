#pragma once

#include "IClient.h"
#include "env.h"

namespace api::v1 {
    class YouTube;

    class YouTubeClient final : public IClient<YouTubeClient, YouTube> {
        static constexpr std::string_view refreshToken = YOUTUBE_REFRESH_TOKEN;
        static constexpr std::string_view clientId = YOUTUBE_CLIENT_ID;
        static constexpr std::string_view clientSecret = YOUTUBE_CLIENT_SECRET;
        static constexpr std::string_view tokenUrl = "https://oauth2.googleapis.com/token";
        std::string auth() const override;
        bool setPostId(const cpr::Response& response, const Json::Value& jsonResponse, YouTube* post) const override;
        bool setAccessToken() override;

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
