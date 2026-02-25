#include "clients/PinterestClient.h"
#include "clients/models/Pin.h"
#include <fmt/chrono.h>

namespace api::v1 {
    std::string PinterestClient::apiUploadMedia = fmt::format("{}/v5/media", getEnv("PINTEREST_API_HOST"));
    std::string PinterestClient::apiCreatePost = fmt::format("{}/v5/pins", getEnv("PINTEREST_API_HOST"));
    std::string PinterestClient::tokenUrl = fmt::format("{}/v5/oauth/token", getEnv("PINTEREST_API_HOST"));

    std::string PinterestClient::auth() const {
        return fmt::format("Bearer {}", accessToken);
    }

    std::string PinterestClient::getAccessToken() const {
        return accessToken;
    }

    bool PinterestClient::setAccessToken() {
        if(!accessToken.empty())
            return true;

        const auto dbClient = drogon::app().getDbClient("default_not_fast");
        if(!dbClient) {
            sentryHelper(std::runtime_error("DB client not available"), "PinterestClient::setAccessToken");
            return false;
        }

        try {
            const auto result = dbClient->execSqlSync(
                "SELECT access_token, refresh_token, "
                "access_token_expires_at > NOW() AS access_valid, "
                "refresh_token_expires_at > NOW() AS refresh_valid "
                "FROM pinterest_token LIMIT 1");

            if(result.empty()) {
                sentryHelper(std::runtime_error("Pinterest token not found — run the OAuth flow at /admin/pinterest/oauth"),
                             "PinterestClient::setAccessToken");
                return false;
            }

            const auto &row = result[0];

            if(row["access_valid"].as<bool>()) {
                accessToken = row["access_token"].as<std::string>();
                return true;
            }

            if(!row["refresh_valid"].as<bool>()) {
                sentryHelper(std::runtime_error("Pinterest refresh token expired — re-run the OAuth flow at /admin/pinterest/oauth"),
                             "PinterestClient::setAccessToken");
                return false;
            }

            // Access token expired but refresh token still valid — exchange it
            const std::string storedRefreshToken = row["refresh_token"].as<std::string>();
            cpr::Payload payload{{"refresh_token", storedRefreshToken}, {"grant_type", "refresh_token"}};
            const cpr::Response response =
                Post(cpr::Url{tokenUrl},
                     std::move(payload),
                     cpr::Header{{"Content-Type", "application/x-www-form-urlencoded"},
                                 {"Authorization",
                                  "Basic " + Base64::Encode(std::format("{}:{}", clientId, clientSecret))}});

            if(!checkResponses(std::vector{response}))
                return false;

            Json::Value jsonResponse;
            if(!parseJson(response, jsonResponse) || !fieldIsMember("access_token", response, jsonResponse))
                return false;

            accessToken = jsonResponse["access_token"].asString();
            const int expiresIn = jsonResponse.get("expires_in", 2592000).asInt();
            const auto now = std::chrono::system_clock::now();
            const auto newExpiry = now + std::chrono::seconds(expiresIn);
            const auto newExpiryT = std::chrono::system_clock::to_time_t(newExpiry);
            struct tm newExpiryTm{};
            gmtime_r(&newExpiryT, &newExpiryTm);
            const std::string newExpiryStr = fmt::format("{:%Y-%m-%d %H:%M:%S}", newExpiryTm);

            dbClient->execSqlSync(
                "UPDATE pinterest_token SET access_token = $1, access_token_expires_at = $2, updated_at = NOW() WHERE singleton = TRUE",
                accessToken,
                newExpiryStr);

            return true;
        } catch(const drogon::orm::DrogonDbException &e) {
            sentryHelper(std::runtime_error(e.base().what()), "PinterestClient::setAccessToken");
            return false;
        }
    }

    bool PinterestClient::uploadVideos(const Pin* pin) const {
        if(accessToken.empty()) {
            sentryHelper(std::runtime_error("Pinterest Access token is empty"), "PinterestClient::uploadVideos");
            return false;
        }

        if(pin->videos.empty())
            return false;

        cpr::MultiPerform multiplePerform;

        std::vector<std::shared_ptr<cpr::Session>> sessionsInit;
        sessionsInit.reserve(pin->videos.size());

        std::ranges::transform(pin->videos,
                               std::back_inserter(sessionsInit),
                               [this, &multiplePerform]([[maybe_unused]] const auto& media) {
                                   auto session = std::make_shared<cpr::Session>();
                                   session->SetUrl(cpr::Url{apiUploadMedia});
                                   session->SetHeader(getHttpHeaders());
                                   session->SetBody(cpr::Body{R"({"media_type":"video"})"});
                                   multiplePerform.AddSession(session);
                                   return session;
                               });
        if(const auto responses = multiplePerform.Post();
           !checkResponses(responses, 201) || !saveMediaIdString(responses, pin->videos))
            return false;

        cpr::MultiPerform multipleUpload;
        std::vector<std::shared_ptr<cpr::Session>> sessionsUpload;
        sessionsUpload.reserve(pin->videos.size());

        std::ranges::transform(pin->videos,
                               std::back_inserter(sessionsUpload),
                               [this, &multipleUpload](const auto& media) {
                                   auto session = std::make_shared<cpr::Session>();
                                   const auto& response = media->getResponse();
                                   session->SetUrl(cpr::Url{response["upload_url"].asString()});

                                   cpr::Multipart multipart{};

                                   const auto& uploadParams = response["upload_parameters"];
                                   for(const auto& paramName: uploadParams.getMemberNames()) {
                                       multipart.parts.emplace_back(paramName, uploadParams[paramName].asString());
                                   }
                                   multipart.parts.emplace_back("file", cpr::File{media->getFileName()});
                                   session->SetMultipart(multipart);
                                   multipleUpload.AddSession(session);
                                   return session;
                               });
        return checkResponses(multipleUpload.Post(), 204);
    }

    bool PinterestClient::setPostId(const cpr::Response& response, const Json::Value& jsonResponse, Pin* pin) const {
        if(!fieldIsMember("id", response, jsonResponse))
            return false;
        pin->postId = jsonResponse["id"].asString();
        return true;
    }
}
