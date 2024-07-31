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
#include "HttpException.h"
#include <functional>
#include <memory>
#include <curl/curl.h>
#include <uuid/uuid.h>

std::unique_ptr<TwitterClient> TwitterClient::instance = nullptr;

static size_t WriteCallback(const char* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append(contents, size * nmemb);
    return size * nmemb;
}

static size_t WriteCallbackToFile(const char* contents, size_t size, size_t nmemb, std::ofstream* ofs) {
    ofs->write(contents, size * nmemb);
    return size * nmemb;
}

std::string urlEncode(const std::string& value) {
    std::string escaped;
    for(char c: value) {
        if(isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped += c;
        } else {
            escaped += fmt::format("%{:02X}", static_cast<unsigned int>(static_cast<unsigned char>(c)));
        }
    }
    return escaped;
}

std::string calculateOAuthSignature(const std::string& httpMethod,
                                    const std::string& url,
                                    const std::map<std::string, std::string, std::less<>>& params,
                                    const std::string& consumerSecret,
                                    const std::string& tokenSecret) {
    // Step 1-5: Create the signature base string
    std::vector<std::string> encodedParams;
    std::ranges::transform(params, std::back_inserter(encodedParams), [](const auto& pair) {
        return fmt::format("{}={}", urlEncode(pair.first), urlEncode(pair.second));
    });
    std::string paramString = fmt::to_string(fmt::join(encodedParams, "&"));

    std::string signatureBaseString =
        fmt::format("{}&{}&{}", urlEncode(httpMethod), urlEncode(url), urlEncode(paramString));

    // Step 6: Generate the signing key
    std::string signingKey = fmt::format("{}&{}", urlEncode(consumerSecret), urlEncode(tokenSecret));

    // Step 7-8: Sign the base string using HMAC-SHA1 and encode in base64
    unsigned int digest_len;
    const unsigned char* digest = HMAC(EVP_sha1(),
                                       signingKey.c_str(),
                                       static_cast<int>(signingKey.size()),
                                       reinterpret_cast<const unsigned char*>(signatureBaseString.c_str()),
                                       signatureBaseString.length(),
                                       nullptr,
                                       &digest_len);

    // Convert the HMAC digest into base64
    BIO* bmem = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    bmem = BIO_push(b64, bmem);

    BIO_write(bmem, digest, static_cast<int>(digest_len));
    BIO_flush(bmem);

    BUF_MEM* bptr;
    BIO_get_mem_ptr(bmem, &bptr);

    std::string oauthSignature(bptr->data, bptr->length - 1);  // Exclude the null terminator added by BIO

    BIO_free_all(bmem);

    return oauthSignature;
}

std::string generateNonce() {
    uuid_t uuid;
    std::string uuid_str(36, '\0');  // Initialize string with 36 null characters

    // Generate a UUID
    uuid_generate(uuid);
    uuid_unparse(uuid, &uuid_str[0]);

    return uuid_str;
}

bool TwitterClient::addEasyHandleDownload(CurlMultiHandle multi_handle, FileTransferInfo& info) const {
    info.easy_handle = curl_easy_init();

    if(!info.easy_handle) {
        return false;
    }

    curl_easy_setopt(info.easy_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

    curl_easy_setopt(info.easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(info.easy_handle, CURLOPT_DEFAULT_PROTOCOL, "https");
    curl_easy_setopt(info.easy_handle, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(info.easy_handle, CURLOPT_URL, info.url.c_str());
    curl_easy_setopt(info.easy_handle, CURLOPT_WRITEFUNCTION, WriteCallbackToFile);
    curl_easy_setopt(info.easy_handle, CURLOPT_WRITEDATA, info.ofs.get());
    curl_easy_setopt(info.easy_handle, CURLOPT_PRIVATE, &info);
    curl_multi_add_handle(multi_handle, info.easy_handle);
    return true;
}

void TwitterClient::cleanupHandles(CurlMultiHandle multi_handle,
                                   std::vector<FileTransferInfo>& fileTransferInfos) const {
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

bool TwitterClient::multiHandle(CurlMultiHandle multi_handle) const {
    bool success = true;
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

            // Find the corresponding FileTransferInfo object and set its responseCode
            FileTransferInfo* info = nullptr;
            curl_easy_getinfo(easy_handle, CURLINFO_PRIVATE, &info);
            if(info) {
                auto [responseCode, json] = processResponse(easy_handle, CURLE_OK, info->response);
                info->responseCode = responseCode;
                info->externalId = json["media_id_string"].asString();
            }
        }
    }
    return success;
}

std::string TwitterClient::oauth(const std::string& url,
                                 const std::string& method,
                                 const std::map<std::string, std::string, std::less<>>& params) {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto now_in_seconds = std::chrono::duration_cast<std::chrono::seconds>(now).count();
    std::string oauth_timestamp = std::to_string(now_in_seconds);
    std::map<std::string, std::string, std::less<>> oauthParams = {
        {"oauth_consumer_key", apiKey},
        {"oauth_nonce", generateNonce()},
        {"oauth_signature_method", "HMAC-SHA1"},
        {"oauth_timestamp", oauth_timestamp},
        {"oauth_token", accessToken},
        {"oauth_version", "1.0"},
    };
    for(const auto& [key, value]: params) {
        oauthParams[key] = value;
    }

    oauthParams["oauth_signature"] = calculateOAuthSignature(method, url, oauthParams, apiSecretKey, accessTokenSecret);

    std::vector<std::string> parts;
    parts.reserve(oauthParams.size());
    for(const auto& [key, value]: oauthParams) {
        parts.push_back(fmt::format(R"({}="{}")", key, urlEncode(value)));
    }
    return fmt::format("Authorization: OAuth {}", fmt::join(parts.begin(), parts.end(), ", "));
}

bool TwitterClient::addEasyHandleUpload(CurlMultiHandle multi_handle, FileTransferInfo& info) {
    if(info.isVideo()) {
        return addEasyHandleUploadVideo(info);
    }

    std::string url = "https://upload.twitter.com/1.1/media/upload.json";
    std::string httpMethod = "POST";
    info.easy_handle = curl_easy_init();

    if(!info.easy_handle) {
        return false;
    }

    curl_easy_setopt(info.easy_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

    // Check if info.fileName is not empty and the file exists
    if(!info.outputFileName.empty() && std::filesystem::exists(info.outputFileName)) {
        LOG_INFO << "File exists: " << info.outputFileName;
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
        return false;
    }
    struct curl_slist* headers = nullptr;

    std::string oauthData = oauth(url, httpMethod);
    const char* oauthHeader = oauthData.c_str();
    info.headers = curl_slist_append(headers, oauthHeader);

    curl_easy_setopt(info.easy_handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(info.easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(info.easy_handle, CURLOPT_DEFAULT_PROTOCOL, "https");
    curl_easy_setopt(info.easy_handle, CURLOPT_CUSTOMREQUEST, httpMethod.c_str());
    curl_easy_setopt(info.easy_handle, CURLOPT_HTTPPOST, info.post);
    curl_easy_setopt(info.easy_handle, CURLOPT_HTTPHEADER, info.headers);
    curl_easy_setopt(info.easy_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(info.easy_handle, CURLOPT_WRITEDATA, &info.response);
    curl_easy_setopt(info.easy_handle, CURLOPT_PRIVATE, &info);
    curl_multi_add_handle(multi_handle, info.easy_handle);
    return true;
}

std::string TwitterClient::createTweetJson(const Tweet& tweet) {
    std::string domain;
    getenv("APP_DOMAIN", domain);
    std::string itemUrl = fmt::format("https://{}/item/{}", domain, tweet.itemSlug);
    Json::Value jsonObj;
    jsonObj["text"] = fmt::format("{}\nExplore #FaithFishArt: Discover and buy inspiring art. Follow for updates! {}",
                                  tweet.title,
                                  itemUrl);
    std::ranges::for_each(tweet.downloads, [&jsonObj](const FileTransferInfo& info) {
        jsonObj["media"]["media_ids"].append(info.externalId);
    });

    // Convert Json::Value object to string
    Json::StreamWriterBuilder builder;
    return Json::writeString(builder, jsonObj);
}

bool TwitterClient::addEasyHandleUploadVideo(FileTransferInfo& info) {
    if(info.outputFileName.empty() || !std::filesystem::exists(info.outputFileName)) {
        std::string error =
            fmt::format("Error: fileName is empty or file does not exist. Filename: {}", info.outputFileName);
        sentryHelper(error, "performPost");
        return false;
    }

    // Step 1: Initialize
    std::string url = "https://upload.twitter.com/1.1/media/upload.json";
    std::map<std::string, std::string, std::less<>> initParams = {
        {"command", "INIT"},
        {"media_type", "video/mp4"},
        {"total_bytes", std::to_string(std::filesystem::file_size(info.outputFileName))},
        {"media_category", "tweet_video"},
    };

    bool result = uploadVideo(url, initParams, info);
    if(!result || info.externalId.empty()) {
        std::string error = fmt::format(
            "Error: Failed to initialize media upload or externalId is empty. responseCode: {}, response: {}",
            info.responseCode,
            info.response);
        sentryHelper(error, "addEasyHandleUploadVideo");
        return false;
    }

    // Step 2: Append
    std::ifstream fileStream(info.outputFileName, std::ios::in | std::ios::binary);
    if(!fileStream) {
        try {
            throw FileOpenException(info.outputFileName);
        } catch(FileOpenException& e) {
            e.printStackTrace(std::cerr);
        }
        return false;
    }

    std::vector<char> buffer(512 * 1024);  // 512 KB buffer
    size_t segmentIndex = 0;
    while(fileStream.read(buffer.data(), buffer.size()) || fileStream.gcount() > 0) {
        std::map<std::string, std::string, std::less<>> appendParams = {
            {"command", "APPEND"},
            {"media_id", info.externalId},
            {"segment_index", std::to_string(segmentIndex)},
        };

        result = uploadVideo(url, appendParams, info, buffer.data(), fileStream.gcount());
        if(!result) {
            std::string error = fmt::format("Error: Failed to append media segment. responseCode: {}, response: {}",
                                            info.responseCode,
                                            info.response);
            sentryHelper(error, "addEasyHandleUploadVideo");
            return false;
        }
        ++segmentIndex;
    }

    // Step 3: Finalize
    std::map<std::string, std::string, std::less<>> finalizeParams = {
        {"command", "FINALIZE"},
        {"media_id", info.externalId},
    };

    return uploadVideo(url, finalizeParams, info);
}

bool TwitterClient::uploadVideo(const std::string& url,
                                const std::map<std::string, std::string, std::less<>>& params,
                                FileTransferInfo& info,
                                const char* fileData,
                                size_t fileSize) {
    info.easy_handle = curl_easy_init();
    std::string httpMethod = "POST";

    if(!info.easy_handle) {
        return false;
    }
    curl_easy_setopt(info.easy_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

    struct curl_slist* headers = nullptr;
    auto it = params.find("command");

    std::string oauthData;
    std::string urlEncodedData;
    curl_httppost* lastptr = nullptr;

    if(it != params.end() && it->second == "APPEND") {
        headers = curl_slist_append(headers, "Content-Type: multipart/form-data");
        oauthData = oauth(url, httpMethod);
        for(const auto& [key, value]: params) {
            curl_formadd(&info.post,
                         &lastptr,
                         CURLFORM_COPYNAME,
                         key.c_str(),
                         CURLFORM_COPYCONTENTS,
                         value.c_str(),
                         CURLFORM_END);
        }
        curl_formadd(&info.post,
                     &lastptr,
                     CURLFORM_COPYNAME,
                     "media",
                     CURLFORM_BUFFER,
                     info.outputFileName.c_str(),
                     CURLFORM_BUFFERPTR,
                     fileData,
                     CURLFORM_BUFFERLENGTH,
                     fileSize,
                     CURLFORM_CONTENTTYPE,
                     "video/mp4",
                     CURLFORM_END);
    } else {
        oauthData = oauth(url, httpMethod, params);
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        std::vector<std::string> encodedParams;
        std::ranges::transform(params, std::back_inserter(encodedParams), [](const auto& pair) {
            LOG_INFO << pair.first << " : " << pair.second;
            return fmt::format("{}={}", urlEncode(pair.first), urlEncode(pair.second));
        });
        urlEncodedData = fmt::to_string(fmt::join(encodedParams, "&"));
    }
    const char* oauthHeader = oauthData.c_str();
    headers = curl_slist_append(headers, oauthHeader);
    info.headers = headers;

    curl_easy_setopt(info.easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(info.easy_handle, CURLOPT_DEFAULT_PROTOCOL, "https");
    curl_easy_setopt(info.easy_handle, CURLOPT_CUSTOMREQUEST, httpMethod.c_str());
    curl_easy_setopt(info.easy_handle, CURLOPT_URL, url.c_str());
    if(it != params.end() && it->second == "APPEND") {
        curl_easy_setopt(info.easy_handle, CURLOPT_HTTPPOST, info.post);
    } else {
        curl_easy_setopt(info.easy_handle, CURLOPT_POSTFIELDS, urlEncodedData.c_str());
    }
    curl_easy_setopt(info.easy_handle, CURLOPT_HTTPHEADER, info.headers);
    curl_easy_setopt(info.easy_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(info.easy_handle, CURLOPT_WRITEDATA, &info.response);
    curl_easy_setopt(info.easy_handle, CURLOPT_PRIVATE, &info);

    CURLcode res = curl_easy_perform(info.easy_handle);

    auto [responseCode, json] = processResponse(info.easy_handle, res, info.response);
    info.responseCode = responseCode;
    curl_easy_cleanup(info.easy_handle);
    info.easy_handle = nullptr;
    curl_formfree(info.post);
    info.post = nullptr;
    curl_slist_free_all(info.headers);
    info.headers = nullptr;

    if(json.isMember("media_id_string")) {
        info.externalId = json["media_id_string"].asString();
        LOG_INFO << "uploadVideo->info.externalId:" << info.externalId;
    }
    if(info.responseCode < 200 || info.responseCode >= 300) {
        return false;
    }
    return true;
}

template<std::predicate<CurlMultiHandle, FileTransferInfo&> TransferFunc>
bool TwitterClient::transMediaFiles(std::vector<FileTransferInfo>& fileTransferInfos,
                                    const TransferFunc& transferFunc) {
    CurlMultiHandle multi_handle = curl_multi_init();
    bool success = true;

    for(auto& info: fileTransferInfos) {
        if(!transferFunc(multi_handle, info)) {
            success = false;
            break;
        }
    }
    if(!success) {
        cleanupHandles(multi_handle, fileTransferInfos);
        return false;
    }
    success = multiHandle(multi_handle);

    for(const auto& info: fileTransferInfos) {
        // Assuming success is indicated by a 200-299 status code
        if(info.responseCode < 200 || info.responseCode >= 300) {
            success = false;
            break;
        }
    }

    cleanupHandles(multi_handle, fileTransferInfos);
    return success;
}

void TwitterClient::performPost(Tweet& tweet) {
    CURL* curl;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
        std::string httpMethod = "POST";
        std::string url = "https://api.twitter.com/2/tweets";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, httpMethod.c_str());

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string oauthData = oauth(url, httpMethod);
        const char* oauthHeader = oauthData.c_str();
        headers = curl_slist_append(headers, oauthHeader);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        std::string jsonData = createTweetJson(tweet);
        const char* data = jsonData.c_str();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        // Set up response handling
        std::string responseString;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        CURLcode res = curl_easy_perform(curl);

        const auto& [responseCode, json] = processResponse(curl, res, responseString);
        if(json.isMember("data") && json["data"].isMember("id")) {
            tweet.tweetId = json["data"]["id"].asString();
        } else {
            std::string error = fmt::format("Error: Failed to post tweet. responseCode: {}, response: {}",
                                            responseCode,
                                            responseString);
            sentryHelper(error, "performPost");
        }

        curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);
}

bool isUrlPath(const std::string& str) {
    // Regular expression to match URL path with query parameters
    std::regex urlPathPattern(R"(^[a-zA-Z0-9_\-]+=[a-zA-Z0-9_\-]+(&[a-zA-Z0-9_\-]+=[a-zA-Z0-9_\-]+)*$)");
    return std::regex_match(str, urlPathPattern);
}

Json::Value splitAndConvertToJson(const std::string& urlPath) {
    if(!isUrlPath(urlPath)) {
        return {};
    }
    std::map<std::string, std::string, std::less<>> params;
    std::istringstream stream(urlPath);
    std::string keyValuePair;

    while(std::getline(stream, keyValuePair, '&')) {
        size_t pos = keyValuePair.find('=');
        if(pos != std::string::npos) {
            std::string key = keyValuePair.substr(0, pos);
            std::string value = keyValuePair.substr(pos + 1);
            params[key] = value;
        }
    }

    Json::Value json;
    for(const auto& [key, value]: params) {
        json[key] = value;
    }

    return json;
}

std::pair<long, Json::Value>
TwitterClient::processResponse(CurlHandle curl, CURLcode res, const std::string& responseString) const {
    Json::Value root;
    char* contentTypeC;
    curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &contentTypeC);
    const std::string contentType = contentTypeC ? contentTypeC : "";
    LOG_INFO << "Response content type: " << contentType;

    long responseCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

    try {
        if(!(res == CURLE_OK && (responseCode >= 200 && responseCode < 300))) {
            throw HttpException(responseCode, responseString);
        }
    } catch(HttpException& e) {
        e.printStackTrace(std::cerr);
    }

    LOG_INFO << "Response :  " << responseString;
    if(!responseString.empty()) {
        if(contentType.find("application/json") != std::string::npos) {
            std::istringstream responseStream(responseString);
            responseStream >> root;
        } else {
            root = splitAndConvertToJson(responseString);
        }
    }

    return {responseCode, root};
}

Json::Value TwitterClient::requestCurl(const std::string& url,
                                       const std::string& httpMethod,
                                       const std::map<std::string, std::string, std::less<>>& oauthParams) {
    CURL* curl = curl_easy_init();

    if(!curl) {
        return "";
    }
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

    std::string oauthData = oauth(url, httpMethod, oauthParams);
    struct curl_slist* headers = nullptr;
    const char* oauthHeader = oauthData.c_str();
    headers = curl_slist_append(headers, oauthHeader);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, httpMethod.c_str());
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Set up response handling
    std::string responseString;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

    CURLcode res = curl_easy_perform(curl);
    const auto& [responseCode, json] = processResponse(curl, res, responseString);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    return json;
}

void TwitterClient::postTweet(Tweet& tweet) {
    if(!transMediaFiles(tweet.downloads, std::bind_front(&TwitterClient::addEasyHandleDownload, this))) {
        LOG_ERROR << "Uploaded media files failed.";
        return;
    }
    LOG_INFO << "Downloaded media files successfully.";

    if(!transMediaFiles(tweet.downloads, std::bind_front(&TwitterClient::addEasyHandleUpload, this))) {
        LOG_ERROR << "Uploaded media files failed.";
        return;
    }
    for(auto const& info: tweet.downloads) {
        std::filesystem::remove(info.outputFileName);
    }
    bool success = true;
    for(auto& info: tweet.downloads) {
        if(info.externalId.empty()) {
            success = false;
            std::string error = fmt::format("Error: externalId is empty. responseCode: {}, response: {}",
                                            info.responseCode,
                                            info.response);
            sentryHelper(error, "performPost");
        }
    }
    if(!success) {
        LOG_INFO << "Uploaded media files failed.";
        return;
    }

    LOG_INFO << "Uploaded media files successfully.";

    performPost(tweet);
}
