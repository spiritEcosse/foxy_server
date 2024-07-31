#include "JWT.h"
#include <curl/curl.h>
#include <fmt/core.h>
#include "src/utils/env.h"

using namespace api::utils::jwt;
using namespace drogon;

size_t WriteCallback(std::any* contents, size_t size, size_t nmemb, const std::string* out) {
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
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            jsonData["error"] = curl_easy_strerror(res);
            httpCode = drogon::k500InternalServerError;
        } else {
            if(Json::Reader jsonReader; jsonReader.parse(readBuffer, jsonData)) {
                std::string aud = jsonData["aud"].asString();
            } else {
                std::cerr << "Failed to parse the JSON response" << std::endl;
                jsonData["error"] = "Failed to parse the JSON response";
                httpCode = drogon::k400BadRequest;
            }
        }
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return {static_cast<drogon::HttpStatusCode>(httpCode), jsonData};
}

JWT JWT::generateToken(const std::map<std::string, ::jwt::traits::kazuho_picojson::value_type, std::less<>>& claims,
                       const bool& extension) {
    const auto time = std::chrono::system_clock::now();

    const int64_t expiresAt = std::chrono::duration_cast<std::chrono::seconds>(
                                  (time + std::chrono::hours{(extension ? 30 : 1) * 24}).time_since_epoch())
                                  .count();

    auto jwtToken = ::jwt::create()
                        .set_type("JWT")
                        .set_issuer(app().getCustomConfig()["jwt"]["issuer"].asString())
                        .set_audience(app().getCustomConfig()["jwt"]["audience"].asString())
                        .set_issued_at(time)
                        .set_not_before(time)
                        .set_expires_at(std::chrono::system_clock::from_time_t(expiresAt));

    for(auto& [key, value]: claims)
        jwtToken.set_payload_claim(key, value);

    return {jwtToken.sign(::jwt::algorithm::hs256{app().getCustomConfig()["jwt"]["private_key"].asString()}),
            expiresAt};
}

std::map<std::string, std::any, std::less<>> JWT::decodeToken(const std::string& encodedToken) {
    try {
        auto decodedToken = ::jwt::decode<::jwt::traits::kazuho_picojson>(encodedToken);

        if(verifyToken(decodedToken)) {
            std::map<std::string, std::any, std::less<>> attributes;

            for(auto const& [key, value]: decodedToken.get_payload_json())
                addClaimToAttributes(attributes, {key, decodedToken.get_payload_claim(key)});

            return attributes;
        }
        return {};
    } catch(const std::invalid_argument& e) {
        return {};
    } catch(const ::jwt::error::signature_verification_exception& e) {
        return {};
    } catch(const std::runtime_error& e) {
        return {};
    }
}

bool JWT::verifyToken(const ::jwt::decoded_jwt<::jwt::traits::kazuho_picojson>& jwt) {
    auto jwtVerifier =
        ::jwt::verify()
            .with_issuer(app().getCustomConfig()["jwt"]["issuer"].asString())
            .with_audience(app().getCustomConfig()["jwt"]["audience"].asString())
            .allow_algorithm(::jwt::algorithm::hs256{app().getCustomConfig()["jwt"]["private_key"].asString()});

    try {
        jwtVerifier.verify(jwt);
        return true;
    } catch(const ::jwt::error::token_verification_exception& e) {
        return false;
    }
}

void JWT::addClaimToAttributes(
    std::map<std::string, std::any, std::less<>>& attributes,
    const std::pair<std::string, ::jwt::basic_claim<::jwt::traits::kazuho_picojson>>& claim) {
    switch(claim.second.get_type()) {
        case ::jwt::json::type::boolean:
            attributes.try_emplace(claim.first, claim.second.as_boolean());
            break;
        case ::jwt::json::type::integer:
            attributes.try_emplace(claim.first, claim.second.as_integer());
            break;
        case ::jwt::json::type::number:
            attributes.try_emplace(claim.first, claim.second.as_number());
            break;
        case ::jwt::json::type::string:
            attributes.try_emplace(claim.first, claim.second.as_string());
            break;
        case ::jwt::json::type::array:
            attributes.try_emplace(claim.first, claim.second.as_array());
            break;
        case ::jwt::json::type::object:
            attributes.try_emplace(claim.first, claim.second);
            break;
        default:
            throw std::bad_cast();
    }
}
