#pragma once

#include "IClient.h"
#include "env.h"

namespace api::v1 {
    class Tweet;

    class TwitterClient final : public IClient<TwitterClient, Tweet> {
        std::string apiKey;
        std::string apiSecretKey;
        std::string accessToken;
        std::string accessTokenSecret;
        std::string bearerToken;

        std::string auth(const std::string_view url,
                         const std::string_view method = "POST",
                         const TransparentMap& params = {}) const;
        std::string auth() const override;
        bool setPostId(const cpr::Response& response, const Json::Value& jsonResponse, Tweet* tweet) const override;
        bool uploadMediaImage(const Tweet* tweet) const;
        bool uploadMediaVideo(const Tweet* tweet) const;

    public:
        static constexpr std::string media_id = "media_id_string";
        static constexpr std::string_view apiUploadMedia = "https://upload.twitter.com/1.1/media/upload.json";
        static constexpr std::string_view apiCreatePost = "https://api.twitter.com/2/tweets";
        static constexpr std::string_view clientName = "Twitter";
        bool uploadMedia(const Tweet* tweet) const;

        TwitterClient() {
            getenv("TWITTER_API_KEY", apiKey);
            getenv("TWITTER_API_SECRET", apiSecretKey);
            getenv("TWITTER_ACCESS_TOKEN", accessToken);
            getenv("TWITTER_ACCESS_TOKEN_SECRET", accessTokenSecret);
            getenv("TWITTER_BEARER_TOKEN", bearerToken);
        }
    };
}
