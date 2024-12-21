#pragma once

#include "IClient.h"
#include "env.h"

namespace api::v1 {
    class Tweet;

    class TwitterClient final : public IClient<TwitterClient, Tweet> {
        static constexpr std::string_view apiKey = TWITTER_API_KEY;
        static constexpr std::string_view apiSecretKey = TWITTER_API_SECRET;
        static constexpr std::string_view accessToken = TWITTER_ACCESS_TOKEN;
        static constexpr std::string_view accessTokenSecret = TWITTER_ACCESS_TOKEN_SECRET;
        static constexpr std::string_view bearerToken = TWITTER_BEARER_TOKEN;

        std::string auth(const std::string_view url,
                         const std::string_view method = "POST",
                         const TransparentMap& params = {}) const;
        std::string auth() const override;
        bool setPostId(const cpr::Response& response, const Json::Value& jsonResponse, Tweet* tweet) const override;
        bool uploadMediaImage(const Tweet* tweet) const;
        bool uploadMediaVideo(const Tweet* tweet) const;

    public:
        [[nodiscard]] bool uploadMedia(const Tweet* tweet) const;

        static constexpr std::string_view field_media_id = "media_id_string";
        static constexpr std::string_view apiUploadMedia = "https://upload.twitter.com/1.1/media/upload.json";
        static constexpr std::string_view apiCreatePost = "https://api.twitter.com/2/tweets";
        static constexpr std::string_view clientName = "Twitter";
    };
}
