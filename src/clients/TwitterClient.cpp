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

std::unique_ptr<TwitterClient> TwitterClient::instance = nullptr;

static size_t WriteCallback(char* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append(contents, size * nmemb);
    return size * nmemb;
}

static size_t WriteCallbackToFile(char* contents, size_t size, size_t nmemb, std::ofstream* ofs) {
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
    std::string paramString;
    for(const auto& p: params) {
        if(!paramString.empty())
            paramString += "&";
        paramString += urlEncode(p.first) + "=" + urlEncode(p.second);
    }

    std::string signatureBaseString = urlEncode(httpMethod) + "&" + urlEncode(url) + "&" + urlEncode(paramString);

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

bool TwitterClient::multiHandle(CURLM* multi_handle) {
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

void TwitterClient::getRequestToken() {
    std::string url = "https://api.twitter.com/oauth/request_token";
    std::string httpMethod = "POST";
    const auto& json = requestCurl(url, httpMethod);
    requestToken = std::make_unique<RequestToken>(RequestToken::fromJson(json));
    LOG_INFO << "Request token: " << requestToken->getOauthCallbackConfirmed() << ", " << requestToken->getOauthToken()
             << ", " << requestToken->getOauthTokenSecret();
}

void TwitterClient::getAccessToken() {
    std::string url = "https://api.twitter.com/oauth/access_token";
    std::string httpMethod = "POST";
    const auto& json = requestCurl(url, httpMethod);
    std::cout << json.toStyledString() << std::endl;
}

//std::string generate_authorize_url() {
//    // https://api.twitter.com/oauth/authorize?oauth_callback=oob&
//    // oauth_consumer_key=qu3DaWRwnA7P0qOgqSPNPImaw&
//    // oauth_nonce=4Clr5asH5XZJ0QLurHgk22PJD0Up0HRiI2odMSePEE&
//    // oauth_signature=2Hxlpwn5IUOwiyNHKpViuzk4Xt0%253D&
//    // oauth_signature_method=HMAC-SHA1&
//    // oauth_timestamp=1721888654&
//    // oauth_token=0oIkKgAAAAABrhbIAAABkOiPxTY
//    // &oauth_version=1.0
//    // Assuming consumer is an instance of a class that handles OAuth and has a method create_signed_request
//    auto request = consumer.create_signed_request("GET", consumer.authorize_path(), pin_auth_parameters());
//    std::string authHeader = request["Authorization"].substr(6);  // Remove "OAuth " prefix
//    std::stringstream params;
//    bool firstParam = true;
//
//    size_t pos = 0;
//    std::string token;
//    while((pos = authHeader.find(", ")) != std::string::npos) {
//        token = authHeader.substr(0, pos);
//        size_t equalsPos = token.find('=');
//        std::string key = token.substr(0, equalsPos);
//        std::string value = token.substr(equalsPos + 2, token.length() - equalsPos - 3);  // Remove quotes
//
//        if(!firstParam) {
//            params << "&";
//        }
//        params << key << "=" << urlencode(value);
//        firstParam = false;
//
//        authHeader.erase(0, pos + 2);
//    }
//    // Process the last (or only) token
//    size_t equalsPos = authHeader.find('=');
//    std::string key = authHeader.substr(0, equalsPos);
//    std::string value = authHeader.substr(equalsPos + 2, authHeader.length() - equalsPos - 3);  // Remove quotes
//    if(!firstParam) {
//        params << "&";
//    }
//    params << key << "=" << urlencode(value);
//
//    return TwurlOptions::base_url + request.path + "?" + params.str();
//}

std::string TwitterClient::oauth(const std::string& url, const std::string& method) {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto now_in_seconds = std::chrono::duration_cast<std::chrono::seconds>(now).count();
    std::string oauth_timestamp = std::to_string(now_in_seconds);
    std::string oauth_token = requestToken->getOauthToken();
    std::string oauth_token_secret = requestToken->getOauthTokenSecret();
    std::map<std::string, std::string, std::less<>> params = {
        {"oauth_consumer_key", apiKey},
        {"oauth_nonce", generateNonce()},
        {"oauth_signature_method", "HMAC-SHA1"},
        {"oauth_timestamp", oauth_timestamp},
        {"oauth_token", oauth_token.empty() ? accessToken : oauth_token},
        {"oauth_version", "1.0"},
    };

    params["oauth_signature"] =
        calculateOAuthSignature(method,
                                url,
                                params,
                                apiSecretKey,
                                oauth_token_secret.empty() ? accessTokenSecret : oauth_token_secret);

    std::vector<std::string> parts;
    parts.reserve(params.size());
    for(const auto& param: params) {
        parts.push_back(fmt::format(R"({}="{}")", param.first, urlEncode(param.second)));
    }
    return fmt::format("Authorization: OAuth {}", fmt::join(parts.begin(), parts.end(), ", "));
}

bool TwitterClient::addEasyHandleUpload(CURLM* multi_handle, FileTransferInfo& info) {
    if(info.isVideo()) {
        return addEasyHandleUploadVideo(info);
    }

    std::string url = "https://upload.twitter.com/1.1/media/upload.json";
    std::string httpMethod = "POST";
    info.easy_handle = curl_easy_init();

    if(!info.easy_handle) {
        return false;
    }
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
        std::string error = fmt::format("Error: Unable to open file. Filename: {}", info.outputFileName);
        sentryHelper(error, "performPost");
        return false;
    }

    std::vector<char> buffer(512 * 1024);  // 512 KB buffer
    size_t segmentIndex = 0;
    while(fileStream.read(buffer.data(), buffer.size()) || fileStream.gcount() > 0) {
        std::map<std::string, std::string, std::less<>> appendParams = {
            {"command", "APPEND"},
            {"media_id", info.externalId},
            {"segment_index", std::to_string(segmentIndex)}};

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
    std::map<std::string, std::string, std::less<>> finalizeParams = {{"command", "FINALIZE"},
                                                                      {"media_id", info.externalId}};

    return uploadVideo(url, finalizeParams, info);
}

bool TwitterClient::uploadVideo(const std::string& url,
                                std::map<std::string, std::string, std::less<>>& params,
                                FileTransferInfo& info,
                                const char* fileData,
                                size_t fileSize) {
    info.easy_handle = curl_easy_init();
    std::string httpMethod = "POST";

    if(!info.easy_handle) {
        curl_easy_cleanup(info.easy_handle);
        return false;
    }

    auto commandIter = params.find("command");

    std::string postData;
    for(const auto& param: params) {
        if(!postData.empty())
            postData += "&";
        postData += urlEncode(param.first) + "=" + urlEncode(param.second);
    }

    struct curl_slist* headers = nullptr;
    std::string oauthData = oauth(url, httpMethod);
    const char* oauthHeader = oauthData.c_str();
    headers = curl_slist_append(headers, oauthHeader);
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    info.headers = headers;

    curl_easy_setopt(info.easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(info.easy_handle, CURLOPT_DEFAULT_PROTOCOL, "https");
    curl_easy_setopt(info.easy_handle, CURLOPT_CUSTOMREQUEST, httpMethod.c_str());
    curl_easy_setopt(info.easy_handle, CURLOPT_URL, url.c_str());
    //    curl_easy_setopt(info.easy_handle, CURLOPT_HTTPPOST, info.post);
    curl_easy_setopt(info.easy_handle, CURLOPT_POSTFIELDS, postData.c_str());
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
    }
    return true;
}

bool TwitterClient::transMediaFiles(std::vector<FileTransferInfo>& fileTransferInfos, TransferFunc transferFunc) {
    CURLM* multi_handle = curl_multi_init();
    bool success;

    for(auto& info: fileTransferInfos) {
        if(!(this->*transferFunc)(multi_handle, info)) {
            success = false;
            break;
        }
    }

    success = multiHandle(multi_handle);

    for(auto& info: fileTransferInfos) {
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
    std::map<std::string, std::string> params;
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
    for(const auto& param: params) {
        json[param.first] = param.second;
    }

    return json;
}

std::pair<long, Json::Value>
TwitterClient::processResponse(CURL* curl, CURLcode res, const std::string& responseString) {
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

Json::Value TwitterClient::requestCurl(const std::string& url, const std::string& httpMethod) {
    CURL* curl = curl_easy_init();

    if(!curl) {
        curl_easy_cleanup(curl);
        return "";
    }

    std::string oauthData = oauth(url, httpMethod);
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
    //    getRequestToken();
    //    requestAccessToken();
    if(!transMediaFiles(tweet.downloads, &TwitterClient::addEasyHandleDownload)) {
        return;
    }
    std::cout << "Downloaded media files successfully." << std::endl;

    if(!transMediaFiles(tweet.downloads, &TwitterClient::addEasyHandleUpload)) {
        return;
    }
    for(auto& info: tweet.downloads) {
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
        return;
    }

    std::cout << "Uploaded media files successfully." << std::endl;

    performPost(tweet);
}
