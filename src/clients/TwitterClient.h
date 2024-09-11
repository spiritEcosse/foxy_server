//
// Created by ihor on 08.07.2024.
//

#ifndef TWITTERCLIENT_H
#define TWITTERCLIENT_H

#include <curl/curl.h>
#include <string>
#include <iostream>
#include <utility>
#include <fstream>
#include <memory>
#include <vector>
#include "env.h"
#include <map>
#include <json/value.h>
#include "FileOpenException.h"
#include "fmt/format.h"
#include <functional>

struct FileTransferInfo {
    std::string url;
    std::string outputFileName;
    std::unique_ptr<std::ofstream> ofs;
    CURL* easy_handle = nullptr;
    struct curl_slist* headers = nullptr;
    struct curl_httppost* post = nullptr;
    std::string response;
    long responseCode = 0;
    std::string externalId;

    FileTransferInfo(std::string url, const std::string& outputFileName) :
        url(std::move(url)), outputFileName(outputFileName),
        ofs(std::make_unique<std::ofstream>(outputFileName, std::ios::binary)) {}

    // Move constructor
    FileTransferInfo(FileTransferInfo&& other) noexcept :
        url(std::move(other.url)), outputFileName(std::move(other.outputFileName)), ofs(std::move(other.ofs)),
        easy_handle(other.easy_handle), headers(other.headers), post(other.post), response(std::move(other.response)),
        responseCode(other.responseCode) {
        other.easy_handle = nullptr;
        other.post = nullptr;
        other.headers = nullptr;
    }

    // Move assignment operator
    FileTransferInfo& operator=(FileTransferInfo&& other) noexcept {
        if(this != &other) {
            url = std::move(other.url);
            outputFileName = std::move(other.outputFileName);
            ofs = std::move(other.ofs);
            easy_handle = other.easy_handle;
            headers = other.headers;
            post = other.post;
            response = std::move(other.response);
            responseCode = other.responseCode;
            other.easy_handle = nullptr;
            other.headers = nullptr;
            other.post = nullptr;
        }
        return *this;
    }

    // function to detect is it video or not
    [[nodiscard]] bool isVideo() const {
        return url.find(".mp4") != std::string::npos;
    }

    // Destructor
    ~FileTransferInfo() {
        if(easy_handle) {
            curl_easy_cleanup(easy_handle);
        }
        if(headers) {
            curl_slist_free_all(headers);
        }
        if(post) {
            curl_formfree(post);
        }
    }

    // Deleted copy constructor and copy assignment operator to prevent copying
    FileTransferInfo(const FileTransferInfo&) = delete;
    FileTransferInfo& operator=(const FileTransferInfo&) = delete;
};

struct Tweet {
    std::string title;
    std::vector<FileTransferInfo> downloads;
    std::string tweetId;
    std::string itemSlug;
    std::vector<std::string> tags;

    Tweet(std::string title,
          std::vector<FileTransferInfo> downloads,
          std::string itemSlug,
          std::vector<std::string> tags = {}) :
        title(std::move(title)), downloads(std::move(downloads)), itemSlug(std::move(itemSlug)), tags(std::move(tags)) {
    }
};

class RequestToken {
private:
    bool oauth_callback_confirmed;
    std::string oauth_token;
    std::string oauth_token_secret;

public:
    // Constructor
    RequestToken(bool callback_confirmed, std::string token, std::string token_secret) :
        oauth_callback_confirmed(callback_confirmed), oauth_token(std::move(token)),
        oauth_token_secret(std::move(token_secret)) {}

    // Getter methods
    [[nodiscard]] bool getOauthCallbackConfirmed() const {
        return oauth_callback_confirmed;
    }

    [[nodiscard]] std::string getOauthToken() const {
        return oauth_token;
    }

    [[nodiscard]] std::string getOauthTokenSecret() const {
        return oauth_token_secret;
    }

    // Static method to parse JSON and create RequestToken instance
    static RequestToken fromJson(const Json::Value& json) {
        bool callback_confirmed = json["oauth_callback_confirmed"].asString() == "true";
        std::string token = json["oauth_token"].asString();
        std::string token_secret = json["oauth_token_secret"].asString();
        return {callback_confirmed, token, token_secret};
    }
};

using CurlMultiHandle = CURLM*;
using CurlHandle = CURLM*;

class TwitterClient {
private:
    static std::unique_ptr<TwitterClient> instance;
    std::string apiKey;
    std::string apiSecretKey;
    std::string accessToken;
    std::string accessTokenSecret;
    std::string bearerToken;
    std::unique_ptr<RequestToken> requestToken;

    TwitterClient() {
        getenv("TWITTER_API_KEY", apiKey);
        getenv("TWITTER_API_SECRET", apiSecretKey);
        getenv("TWITTER_ACCESS_TOKEN", accessToken);
        getenv("TWITTER_ACCESS_TOKEN_SECRET", accessTokenSecret);
        getenv("TWITTER_BEARER_TOKEN", bearerToken);
        requestToken = std::make_unique<RequestToken>("", "", "");
    }

    template<std::predicate<CurlMultiHandle, FileTransferInfo&> TransferFunc>
    bool transMediaFiles(std::vector<FileTransferInfo>& fileTransferInfos, const TransferFunc& transferFunc);
    void cleanupHandles(CurlMultiHandle multi_handle, std::vector<FileTransferInfo>& fileTransferInfos) const;
    bool addEasyHandleUpload(CurlMultiHandle multi_handle, FileTransferInfo& info);
    bool addEasyHandleUploadVideo(FileTransferInfo& info);
    bool addEasyHandleDownload(CurlMultiHandle multi_handle, FileTransferInfo& info) const;
    void performPost(Tweet& tweet);
    bool uploadVideo(const std::string& url,
                     const std::map<std::string, std::string, std::less<>>& params,
                     FileTransferInfo& fileTransferInfo,
                     const char* fileData = nullptr,
                     size_t fileSize = 0);
    std::string oauth(const std::string& url,
                      const std::string& method,
                      const std::map<std::string, std::string, std::less<>>& params = {});
    static std::string createTweetJson(const Tweet& tweet);
    std::pair<long, Json::Value> processResponse(CURL* curl, CURLcode res, const std::string& responseString) const;
    bool multiHandle(CurlMultiHandle multi_handle) const;
    Json::Value requestCurl(const std::string& url,
                            const std::string& method,
                            const std::map<std::string, std::string, std::less<>>& oauthParams = {});

public:
    // Deleted copy constructor and assignment operator
    TwitterClient(const TwitterClient&) = delete;
    TwitterClient& operator=(const TwitterClient&) = delete;

    // Public access method
    static TwitterClient& getInstance() {
        if(!instance) {
            instance = std::unique_ptr<TwitterClient>(new TwitterClient());
        }
        return *instance;
    }

    void postTweet(Tweet& tweet);
};

#endif  //TWITTERCLIENT_H
