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

struct FileTransferInfo {
    std::string url;
    std::string outputFileName;
    std::unique_ptr<std::ofstream> ofs;
    CURL* easy_handle;
    struct curl_slist* headers;
    struct curl_httppost* post;
    std::string response;

    FileTransferInfo(std::string url, const std::string& outputFileName) :
        url(std::move(url)), outputFileName(outputFileName), ofs(new std::ofstream(outputFileName, std::ios::binary)),
        easy_handle(nullptr), headers(nullptr), post(nullptr) {
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

    Tweet(std::string title, std::vector<FileTransferInfo> downloads) :
        title(std::move(title)), downloads(std::move(downloads)) {}
};

class TwitterClient {
private:
    static std::unique_ptr<TwitterClient> instance;
    std::string apiKey;
    std::string apiSecretKey;
    std::string accessToken;
    std::string accessTokenSecret;
    std::string bearerToken;

    // Encapsulate urlencode within TwitterClient to avoid conflicts
    static std::string urlencode(const std::string& value) {
        CURL* curl = curl_easy_init();
        char* output = curl_easy_escape(curl, value.c_str(), value.length());
        std::string encoded = output;
        curl_free(output);
        curl_easy_cleanup(curl);
        return encoded;
    }

    TwitterClient() {
        getenv("TWITTER_API_KEY", apiKey);
        getenv("TWITTER_API_SECRET", apiSecretKey);
        getenv("TWITTER_ACCESS_TOKEN", accessToken);
        getenv("TWITTER_ACCESS_TOKEN_SECRET", accessTokenSecret);
        getenv("TWITTER_BEREAR_TOKEN", bearerToken);
    }

    void uploadMediaFiles(std::vector<FileTransferInfo>& fileTransferInfos);
    void addEasyHandleUpload(CURLM* multi_handle, FileTransferInfo& info);

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
