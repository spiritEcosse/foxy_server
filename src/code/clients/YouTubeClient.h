#pragma once

#include "IClient.h"
#include "env.h"

namespace api::v1 {
    class YouTube;

    class YouTubeClient final : public IClient<YouTubeClient, YouTube> {
        static constexpr std::string_view accessToken = YOUTUBE_ACCESS_TOKEN;
        std::string auth() const override;
        bool setPostId(const cpr::Response& response, const Json::Value& jsonResponse, YouTube* post) const override;

    public:
        [[nodiscard]] bool uploadVideo(YouTube* post) const;

        static constexpr std::string_view field_media_id = "media_id";
        static constexpr std::string_view apiUploadMedia = "https://www.googleapis.com/upload/youtube/v3/videos";
        static constexpr std::string_view apiCreatePost = "";
        static constexpr std::string_view clientName = "YouTube";
    };
}
