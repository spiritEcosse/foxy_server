#include "clients/PinterestClient.h"
#include "clients/models/Pin.h"
#include <fmt/chrono.h>
#include <future>

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

        struct SelectResult {
            bool ok = false;
            std::string error;
            std::string accessToken;
            std::string refreshToken;
            bool accessValid = false;
            bool refreshValid = false;
        };

        auto selectPromise = std::make_shared<std::promise<SelectResult>>();
        auto selectFuture = selectPromise->get_future();

        drogon::app().getLoop()->queueInLoop([selectPromise]() {
            const auto dbClient = drogon::app().getFastDbClient("default");
            if(!dbClient) {
                SelectResult sr;
                sr.error = "DB client not available";
                selectPromise->set_value(std::move(sr));
                return;
            }
            dbClient->execSqlAsync(
                "SELECT access_token, refresh_token, "
                "access_token_expires_at > NOW() AS access_valid, "
                "refresh_token_expires_at > NOW() AS refresh_valid "
                "FROM pinterest_token LIMIT 1",
                [selectPromise](const drogon::orm::Result& result) {
                    SelectResult sr;
                    if(result.empty()) {
                        sr.error = "Pinterest token not found — run the OAuth flow at /admin/pinterest/oauth";
                        selectPromise->set_value(std::move(sr));
                        return;
                    }
                    const auto& row = result[0];
                    sr.ok = true;
                    sr.accessToken = row["access_token"].as<std::string>();
                    sr.refreshToken = row["refresh_token"].as<std::string>();
                    sr.accessValid = row["access_valid"].as<bool>();
                    sr.refreshValid = row["refresh_valid"].as<bool>();
                    selectPromise->set_value(std::move(sr));
                },
                [selectPromise](const drogon::orm::DrogonDbException& e) {
                    SelectResult sr;
                    sr.error = e.base().what();
                    selectPromise->set_value(std::move(sr));
                });
        });

        const auto sr = selectFuture.get();

        if(!sr.ok) {
            sentryHelper(std::runtime_error(sr.error), "PinterestClient::setAccessToken");
            return false;
        }

        if(sr.accessValid) {
            accessToken = sr.accessToken;
            return true;
        }

        if(!sr.refreshValid) {
            sentryHelper(
                std::runtime_error("Pinterest refresh token expired — re-run the OAuth flow at /admin/pinterest/oauth"),
                "PinterestClient::setAccessToken");
            return false;
        }

        // Access token expired but refresh token still valid — exchange it
        cpr::Payload payload{{"refresh_token", sr.refreshToken}, {"grant_type", "refresh_token"}};
        const cpr::Response response = Post(
            cpr::Url{tokenUrl},
            std::move(payload),
            cpr::Header{{"Content-Type", "application/x-www-form-urlencoded"},
                        {"Authorization", "Basic " + Base64::Encode(std::format("{}:{}", clientId, clientSecret))}});

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

        const std::string newAccessToken = accessToken;
        auto updatePromise = std::make_shared<std::promise<bool>>();
        auto updateFuture = updatePromise->get_future();

        drogon::app().getLoop()->queueInLoop([updatePromise, newAccessToken, newExpiryStr]() {
            const auto dbClient = drogon::app().getFastDbClient("default");
            if(!dbClient) {
                updatePromise->set_value(false);
                return;
            }
            dbClient->execSqlAsync(
                "UPDATE pinterest_token SET access_token = $1, access_token_expires_at = $2, "
                "updated_at = NOW() WHERE singleton = TRUE",
                [updatePromise](const drogon::orm::Result&) {
                    updatePromise->set_value(true);
                },
                [updatePromise](const drogon::orm::DrogonDbException& e) {
                    sentryHelper(std::runtime_error(e.base().what()), "PinterestClient::setAccessToken");
                    updatePromise->set_value(false);
                },
                newAccessToken,
                newExpiryStr);
        });

        return updateFuture.get();
    }

    bool PinterestClient::uploadVideos(const Pin* pin) const {
        if(accessToken.empty()) {
            sentryHelper(std::runtime_error("Pinterest Access token is empty"), "PinterestClient::uploadVideos");
            return false;
        }

        if(pin->videos.empty())
            return false;

        cpr::MultiPerform multiplePerform;

        auto sessionsInit = pin->videos |
                            std::views::transform([this, &multiplePerform]([[maybe_unused]] const auto& media) {
                                auto session = std::make_shared<cpr::Session>();
                                session->SetUrl(cpr::Url{apiUploadMedia});
                                session->SetHeader(getHttpHeaders());
                                session->SetBody(cpr::Body{R"({"media_type":"video"})"});
                                multiplePerform.AddSession(session);
                                return session;
                            }) |
                            std::ranges::to<std::vector>();
        if(const auto responses = multiplePerform.Post();
           !checkResponses(responses, 201) || !saveMediaIdString(responses, pin->videos))
            return false;

        cpr::MultiPerform multipleUpload;
        auto sessionsUpload = pin->videos | std::views::transform([this, &multipleUpload](const auto& media) {
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
                              }) |
                              std::ranges::to<std::vector>();
        return checkResponses(multipleUpload.Post(), 204);
    }

    bool PinterestClient::setPostId(const cpr::Response& response, const Json::Value& jsonResponse, Pin* pin) const {
        if(!fieldIsMember("id", response, jsonResponse))
            return false;
        pin->postId = jsonResponse["id"].asString();
        return true;
    }
}
