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

struct FileTransferInfo {
    std::string url;
    std::string outputFileName;
    std::unique_ptr<std::ofstream> ofs;
    CURL* easy_handle;
    struct curl_slist* headers;
    struct curl_httppost* post;
    std::string response;
    long responseCode;
    std::string externalId;

    FileTransferInfo(std::string url, const std::string& outputFileName) :
        url(std::move(url)), outputFileName(outputFileName), ofs(new std::ofstream(outputFileName, std::ios::binary)),
        easy_handle(nullptr), headers(nullptr), post(nullptr), responseCode(0) {
        if(!ofs->is_open()) {
            throw std::runtime_error("Failed to open file: " + outputFileName);
        }
    }

    // Move constructor
    FileTransferInfo(FileTransferInfo&& other) noexcept :
        url(std::move(other.url)), outputFileName(std::move(other.outputFileName)), ofs(std::move(other.ofs)),
        easy_handle(other.easy_handle), headers(other.headers), post(other.post), response(std::move(other.response)) {
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
            other.easy_handle = nullptr;
            other.headers = nullptr;
            other.post = nullptr;
        }
        return *this;
    }

    // function to detect is it video or not
    bool isVideo() {
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

    Tweet(std::string title, std::vector<FileTransferInfo> downloads, std::string itemSlug) :
        title(std::move(title)), downloads(std::move(downloads)), itemSlug(std::move(itemSlug)) {}
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

    using CurlMultiHandle = CURLM*;
    using TransferFunc = std::function<bool(CurlMultiHandle, FileTransferInfo&)>;

    bool transMediaFiles(std::vector<FileTransferInfo>& fileTransferInfos, const TransferFunc& transferFunc);
    void cleanupHandles(CurlMultiHandle multi_handle, std::vector<FileTransferInfo>& fileTransferInfos);
    bool addEasyHandleUpload(CurlMultiHandle multi_handle, FileTransferInfo& info);
    bool addEasyHandleUploadVideo(FileTransferInfo& info);
    bool addEasyHandleDownload(CurlMultiHandle multi_handle, FileTransferInfo& info);
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
    std::pair<long, Json::Value> processResponse(CURL* curl, CURLcode res, const std::string& responseString);
    bool multiHandle(CurlMultiHandle multi_handle);
    Json::Value requestCurl(const std::string& url,
                            const std::string& method,
                            const std::map<std::string, std::string, std::less<>>& oauthParams = {});
    bool getRequestToken();
    bool getAccessToken(const std::string& pin);
    std::string authenticate();

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
