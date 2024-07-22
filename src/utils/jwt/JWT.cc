#include "JWT.h"
#include <curl/curl.h>
#include <fmt/core.h>
#include "env.h"
#include "sentryHelper.h"

using namespace api::utils::jwt;
using namespace drogon;

size_t WriteCallback(std::any* contents, size_t size, size_t nmemb, std::string* out) {
    size_t totalSize = size * nmemb;
    out->append((char*)contents, totalSize);
    return totalSize;
}

std::tuple<drogon::HttpStatusCode, Json::Value> JWT::verifyGoogleToken(const std::string& token) {
    CURL* curl;
    std::string readBuffer;
    long httpCode = drogon::k500InternalServerError;
    CURLcode res = CURLE_INTERFACE_FAILED;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    Json::Value jsonData;

    if(curl) {
        std::string url = fmt::format("https://oauth2.googleapis.com/tokeninfo?id_token={}", token);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        std::string env;
        getenv("ENV", env);
        if(env == "dev") {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        }
        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        if(res != CURLE_OK) {
            std::string error = fmt::format("curl_easy_perform() failed: ", curl_easy_strerror(res));
            sentryHelper(error, "verifyGoogleToken");
            jsonData["error"] = curl_easy_strerror(res);
            httpCode = drogon::k500InternalServerError;
        }
        Json::Reader jsonReader;
        jsonReader.parse(readBuffer, jsonData);
        if(jsonData.empty()) {
            jsonData["error"] = "Failed to parse the JSON response";
        }
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return {static_cast<drogon::HttpStatusCode>(httpCode), std::move(jsonData)};
}
