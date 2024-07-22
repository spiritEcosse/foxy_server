//
// Created by ihor on 08.07.2024.
//

#include "TwitterClient.h"
#include "fmt/format.h"
#include <string>
#include <random>
#include <openssl/buffer.h>
#include <sstream>
#include <iomanip>
#include <openssl/hmac.h>
#include <map>
#include <drogon/drogon.h>
#include <fstream>
#include <sentry.h>
#include "sentryHelper.h"

std::unique_ptr<TwitterClient> TwitterClient::instance = nullptr;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static size_t WriteCallbackToFile(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* ofs = static_cast<std::ofstream*>(userp);
    ofs->write(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

std::string urlEncode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for(char c: value) {
        if(isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << std::uppercase;
            escaped << '%' << std::setw(2) << int((unsigned char)c);
            escaped << std::nouppercase;
        }
    }

    return escaped.str();
}

std::string calculateOAuthSignature(const std::string& httpMethod,
                                    const std::string& baseUrl,
                                    const std::map<std::string, std::string>& params,
                                    const std::string& consumerSecret,
                                    const std::string& tokenSecret) {
    // Step 1-5: Create the signature base string
    std::string paramString;
    for(const auto& p: params) {
        if(!paramString.empty())
            paramString += "&";
        paramString += urlEncode(p.first) + "=" + urlEncode(p.second);
    }

    std::string signatureBaseString = urlEncode(httpMethod) + "&" + urlEncode(baseUrl) + "&" + urlEncode(paramString);

    // Step 6: Generate the signing key
    std::string signingKey = urlEncode(consumerSecret) + "&" + urlEncode(tokenSecret);

    // Step 7-8: Sign the base string using HMAC-SHA1 and encode in base64
    unsigned char* digest;
    unsigned int digest_len;
    digest = HMAC(EVP_sha1(),
                  signingKey.c_str(),
                  signingKey.length(),
                  reinterpret_cast<const unsigned char*>(signatureBaseString.c_str()),
                  signatureBaseString.length(),
                  nullptr,
                  &digest_len);

    // Convert the HMAC digest into base64
    BIO* bmem = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    bmem = BIO_push(b64, bmem);

    BIO_write(bmem, digest, digest_len);
    BIO_flush(bmem);

    BUF_MEM* bptr;
    BIO_get_mem_ptr(bmem, &bptr);

    std::string oauthSignature(bptr->data, bptr->length - 1);  // Exclude the null terminator added by BIO

    BIO_free_all(bmem);

    return oauthSignature;
}

std::string generateNonce(size_t length = 32) {
    // Characters to generate random part of the nonce
    const char charset[] = "0123456789"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz";
    const size_t charsetSize = sizeof(charset) - 1;

    // Seed with a real random value, if available
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<> dist(0, charsetSize - 1);

    // Start with the current timestamp
    auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    std::string nonce = std::to_string(timestamp);

    // Append random characters to the nonce
    for(size_t i = 0; i < length; ++i) {
        nonce += charset[dist(rng)];
    }

    return nonce;
}

bool TwitterClient::addEasyHandleDownload(CURLM* multi_handle, FileTransferInfo& info) {
    info.easy_handle = curl_easy_init();
    if(!info.easy_handle) {
        return false;
    }
    curl_easy_setopt(info.easy_handle, CURLOPT_URL, info.url.c_str());
    curl_easy_setopt(info.easy_handle, CURLOPT_WRITEFUNCTION, WriteCallbackToFile);
    curl_easy_setopt(info.easy_handle, CURLOPT_WRITEDATA, info.ofs.get());
    curl_easy_setopt(info.easy_handle, CURLOPT_PRIVATE, info.easy_handle);
    curl_multi_add_handle(multi_handle, info.easy_handle);
    return true;
}

void cleanupHandles(CURLM* multi_handle, std::vector<FileTransferInfo>& fileTransferInfos) {
    for(auto& info: fileTransferInfos) {
        if(info.easy_handle) {
            curl_multi_remove_handle(multi_handle, info.easy_handle);
            curl_easy_cleanup(info.easy_handle);
            info.easy_handle = nullptr;
        }

        if(info.headers) {
            curl_slist_free_all(info.headers);
            info.headers = nullptr;
        }

        if(info.post) {
            curl_formfree(info.post);
            info.post = nullptr;
        }

        if(info.ofs) {
            info.ofs->close();
        }
    }
    curl_multi_cleanup(multi_handle);
}

void multiHandle(CURLM* multi_handle) {
    int still_running;
    curl_multi_perform(multi_handle, &still_running);

    while(still_running) {
        int numfds;
        curl_multi_wait(multi_handle, nullptr, 0, 1000, &numfds);
        curl_multi_perform(multi_handle, &still_running);
    }

    int msgs_left;
    CURLMsg* msg;
    while((msg = curl_multi_info_read(multi_handle, &msgs_left))) {
        if(msg->msg == CURLMSG_DONE) {
            CURL* easy_handle = msg->easy_handle;
            char* url;
            curl_easy_getinfo(easy_handle, CURLINFO_EFFECTIVE_URL, &url);
            long response_code;
            curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &response_code);
            std::string* responseString = nullptr;
            curl_easy_getinfo(easy_handle, CURLINFO_PRIVATE, &responseString);

            if(response_code < 200 || response_code >= 300) {
                std::string error = fmt::format("response_code: {}, response: {}",
                                                response_code,
                                                responseString ? *responseString : "No response string");
                sentryHelper(error, "multiHandle");
            }
        }
    }
}

bool TwitterClient::addEasyHandleUpload(CURLM* multi_handle, FileTransferInfo& info) {
    std::string url = "https://upload.twitter.com/1.1/media/upload.json";
    info.easy_handle = curl_easy_init();

    if(!info.easy_handle) {
        return false;
    }
    // Check if info.fileName is not empty and the file exists
    if(!info.outputFileName.empty() && std::filesystem::exists(info.outputFileName)) {
        curl_formadd(&info.post,
                     &info.post,
                     CURLFORM_COPYNAME,
                     "media",
                     CURLFORM_FILE,
                     info.outputFileName.c_str(),
                     CURLFORM_END);
    } else {
        std::string error =
            fmt::format("Error: fileName is empty or file does not exist. Filename: {}", info.outputFileName);
        sentryHelper(error, "performPost");
    }
    struct curl_slist* headers = nullptr;
    std::ostringstream authHeader;
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto now_in_seconds = std::chrono::duration_cast<std::chrono::seconds>(now).count();
    std::string oauth_timestamp = std::to_string(now_in_seconds);
    std::map<std::string, std::string> params = {{"oauth_consumer_key", apiKey},
                                                 {"oauth_nonce", generateNonce()},
                                                 {"oauth_signature_method", "HMAC-SHA1"},
                                                 {"oauth_timestamp", oauth_timestamp},
                                                 {"oauth_token", accessToken},
                                                 {"oauth_version", "1.0"}};

    std::string signature = calculateOAuthSignature("POST", url, params, apiSecretKey, accessTokenSecret);
    params["oauth_signature"] = signature;

    authHeader << "OAuth ";
    for(auto it = params.begin(); it != params.end(); ++it) {
        if(it != params.begin()) {
            authHeader << ", ";
        }
        authHeader << it->first << "=\"" << urlEncode(it->second) << "\"";
    }

    headers = curl_slist_append(headers, ("Authorization: " + authHeader.str()).c_str());
    info.headers = headers;

    curl_easy_setopt(info.easy_handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(info.easy_handle, CURLOPT_HTTPPOST, info.post);
    curl_easy_setopt(info.easy_handle, CURLOPT_HTTPHEADER, info.headers);
    curl_easy_setopt(info.easy_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(info.easy_handle, CURLOPT_WRITEDATA, &info.response);
    curl_multi_add_handle(multi_handle, info.easy_handle);
    return true;
}

bool TwitterClient::transMediaFiles(std::vector<FileTransferInfo>& fileTransferInfos, TransferFunc transferFunc) {
    CURLM* multi_handle = curl_multi_init();
    bool success = true;

    for(auto& info: fileTransferInfos) {
        if(!(this->*transferFunc)(multi_handle, info)) {
            success = false;
            break;
        }
    }

    multiHandle(multi_handle);
    cleanupHandles(multi_handle, fileTransferInfos);
    return success;
}

void TwitterClient::performPost(Tweet& tweet) {
    CURL* curl;
    curl = curl_easy_init();
    if(curl) {
        std::string httpMethod = "POST";
        std::string baseUrl = "https://api.twitter.com/2/tweets";
        curl_easy_setopt(curl, CURLOPT_URL, baseUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        auto now = std::chrono::system_clock::now().time_since_epoch();
        auto now_in_seconds = std::chrono::duration_cast<std::chrono::seconds>(now).count();
        std::string oauth_timestamp = std::to_string(now_in_seconds);
        std::map<std::string, std::string> params = {{"oauth_consumer_key", apiKey},
                                                     {"oauth_token", accessToken},
                                                     {"oauth_signature_method", "HMAC-SHA1"},
                                                     {"oauth_timestamp", oauth_timestamp},
                                                     {"oauth_nonce", generateNonce()},
                                                     {"oauth_version", "1.0"}};

        std::string signature = calculateOAuthSignature(httpMethod, baseUrl, params, apiSecretKey, accessTokenSecret);

        // Add the signature to the params map
        params["oauth_signature"] = urlEncode(signature);

        // Construct the Authorization header
        std::string authHeader = "Authorization: OAuth ";
        for(const auto& param: params) {
            authHeader += param.first + "=\"" + param.second + "\", ";
        }
        authHeader.pop_back();
        authHeader.pop_back();

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::string domain;
        getenv("APP_DOMAIN", domain);
        std::string url = fmt::format("https://{}/item/{}", domain, tweet.itemSlug);
        Json::Value jsonObj;
        jsonObj["text"] =
            fmt::format("{}\nExplore #FaithFishArt: Discover and buy inspiring art. Follow for updates! {}",
                        tweet.title,
                        url);
        std::for_each(tweet.downloads.begin(), tweet.downloads.end(), [&jsonObj](const auto& info) {
            Json::Value root;
            std::istringstream responseStream(info.response);
            responseStream >> root;
            jsonObj["media"]["media_ids"].append(root["media_id_string"].asString());
        });

        // Convert Json::Value object to string
        Json::StreamWriterBuilder builder;
        std::string dataStr = Json::writeString(builder, jsonObj);
        const char* data = dataStr.c_str();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        // Set up response handling
        std::string responseString;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        CURLcode res = curl_easy_perform(curl);
        long response_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if(res == CURLE_OK && (response_code >= 200 && response_code < 300)) {
            // Success: handle successful response
            Json::Value root;
            std::istringstream responseStream(responseString);
            responseStream >> root;
            tweet.tweetId = root["data"]["id"].asString();
        } else if(res == CURLE_OK) {
            // add here response
            std::string error = fmt::format("response_code: {}, response: {}", response_code, responseString);
            sentryHelper(error, "performPost");
        } else {
            // A transport level error occurred
            sentryHelper(curl_easy_strerror(res), "performPost");
        }

        curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);
}

void TwitterClient::postTweet(Tweet& tweet) {
    bool downloadSuccess = transMediaFiles(tweet.downloads, &TwitterClient::addEasyHandleDownload);
    if(!downloadSuccess) {
        std::string error = "Failed to download media files.";
        sentryHelper(error, "download, postTweet");
        return;
    }

    bool uploadSuccess = transMediaFiles(tweet.downloads, &TwitterClient::addEasyHandleUpload);
    if(!uploadSuccess) {
        std::string error = "Failed to upload media files.";
        sentryHelper(error, "upload, postTweet");
        return;  // Exit if uploading media files failed
    }

    performPost(tweet);
}
