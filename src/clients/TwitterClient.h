#pragma once

#include "IClient.h"
#include "env.h"

namespace api::v1 {
    class Tweet;

    class StringViewHasher final : public BaseClass {
    public:
        // Support hashing for std::string, std::string_view, and const char*
        using is_transparent = void;  // This signals that the hasher is "transparent"

        // Hash for std::string_view (covers std::string as well)
        size_t operator()(const std::string_view key) const noexcept {
            return std::hash<std::string_view>{}(key);
        }

        // Hash for const char* (c-string)
        size_t operator()(const char* key) const noexcept {
            return std::hash<std::string_view>{}(key);
        }
    };

    class TwitterClient final : public IClient<TwitterClient, Tweet> {
        std::string apiKey;
        std::string apiSecretKey;
        std::string accessToken;
        std::string accessTokenSecret;
        std::string bearerToken;

        std::string
        auth(const std::string_view url,
             const std::string_view method = "POST",
             const std::unordered_map<std::string, std::string, StringViewHasher, std::less<>>& params = {});
        std::string auth() override;
        bool uploadMediaImage(const Tweet* tweet);
        bool uploadMediaVideo(const Tweet* tweet);
        static bool saveMediaIdString(const std::vector<cpr::Response>& responses,
                                      const std::vector<SharedFileTransferInfo>& medias);

    public:
        static constexpr std::string_view apiUploadMedia = "https://upload.twitter.com/1.1/media/upload.json";
        static constexpr std::string_view apiCreatePost = "https://api.twitter.com/2/tweets";
        static constexpr std::string_view clientName = "Twitter";
        bool uploadMedia(const Tweet* tweet);

        TwitterClient() {
            getenv("TWITTER_API_KEY", apiKey);
            getenv("TWITTER_API_SECRET", apiSecretKey);
            getenv("TWITTER_ACCESS_TOKEN", accessToken);
            getenv("TWITTER_ACCESS_TOKEN_SECRET", accessTokenSecret);
            getenv("TWITTER_BEARER_TOKEN", bearerToken);
        }
    };
}
