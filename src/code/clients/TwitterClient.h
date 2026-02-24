#pragma once

#include "clients/IClient.h"
#include "utils/config.h"
#include "utils/TransparentStringHash.h"

namespace api::v1 {
    class Tweet;

    class TwitterClient final : public IClient<TwitterClient, Tweet> {
        static inline const std::string apiKey = getEnv("TWITTER_API_KEY");
        static inline const std::string apiSecretKey = getEnv("TWITTER_API_SECRET");
        static inline const std::string accessToken = getEnv("TWITTER_ACCESS_TOKEN");
        static inline const std::string accessTokenSecret = getEnv("TWITTER_ACCESS_TOKEN_SECRET");
        static inline const std::string bearerToken = getEnv("TWITTER_BEARER_TOKEN");

        std::string auth(const std::string_view url,
                         const std::string_view method = "POST",
                         const TransparentMap& params = {}) const;
        std::string auth() const override;
        bool setPostId(const cpr::Response& response, const Json::Value& jsonResponse, Tweet* tweet) const override;
        bool uploadMediaImage(const Tweet* tweet) const;
        bool uploadMediaVideo(const Tweet* tweet) const;
        bool setAccessToken() override;
        std::string getAccessToken() const override;

    public:
        [[nodiscard]] bool uploadMedia(const Tweet* tweet) const;

        static constexpr std::string_view field_media_id = "media_id_string";
        static constexpr std::string_view apiUploadMedia = "https://upload.twitter.com/1.1/media/upload.json";
        static constexpr std::string_view apiCreatePost = "https://api.twitter.com/2/tweets";
        static constexpr std::string_view clientName = "Twitter";
    };
}
