#include "controllers/PinterestOAuth.h"
#include "models/PinterestTokenModel.h"
#include "utils/config.h"
#include "utils/Base64.h"
#include "sentry_catcher/sentryHelper.h"
#include <cpr/cpr.h>
#include <json/json.h>
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <drogon/utils/Utilities.h>
#include <chrono>
#include <future>
#include <optional>
#include <thread>

using namespace api::v1;
using namespace drogon;

namespace {
    std::string formatExpiry(std::chrono::system_clock::time_point tp) {
        const auto t = std::chrono::system_clock::to_time_t(tp);
        struct tm gmt{};
        gmtime_r(&t, &gmt);
        return fmt::format("{:%Y-%m-%dT%H:%M:%SZ}", gmt);
    }

    void exchangeTokenAndSave(const std::shared_ptr<std::function<void(const HttpResponsePtr &)>> &callbackPtr,
                              const std::string &code) {
        const std::string clientId = getEnv("PINTEREST_CLIENT_ID");
        const std::string clientSecret = getEnv("PINTEREST_CLIENT_SECRET");
        const std::string apiHost = getEnv("PINTEREST_API_HOST");
        const std::string redirectUri =
            getEnv("PINTEREST_REDIRECT_URI", "http://localhost:8080/admin/pinterest/oauth/callback");
        const std::string tokenUrl = fmt::format("{}/v5/oauth/token", apiHost);

        const cpr::Response tokenResponse = cpr::Post(
            cpr::Url{tokenUrl},
            cpr::Payload{{"grant_type", "authorization_code"}, {"code", code}, {"redirect_uri", redirectUri}},
            cpr::Header{{"Content-Type", "application/x-www-form-urlencoded"},
                        {"Authorization", "Basic " + Base64::Encode(fmt::format("{}:{}", clientId, clientSecret))}});

        if(tokenResponse.status_code != 200) {
            sentryHelper(std::runtime_error(fmt::format("Pinterest token exchange failed. Status: {}, Response: {}",
                                                        tokenResponse.status_code,
                                                        tokenResponse.text)),
                         "PinterestOAuth::callback");
            Json::Value json;
            json["error"] = "Token exchange failed";
            json["detail"] = tokenResponse.text;
            const auto resp = HttpResponse::newHttpJsonResponse(std::move(json));
            resp->setStatusCode(HttpStatusCode::k502BadGateway);
            (*callbackPtr)(resp);
            return;
        }

        Json::Value tokenJson;
        if(Json::Reader reader; !reader.parse(tokenResponse.text, tokenJson)) {
            Json::Value json;
            json["error"] = "Failed to parse token response";
            const auto resp = HttpResponse::newHttpJsonResponse(std::move(json));
            resp->setStatusCode(HttpStatusCode::k502BadGateway);
            (*callbackPtr)(resp);
            return;
        }

        const std::string accessToken = tokenJson["access_token"].asString();
        const std::string refreshToken = tokenJson["refresh_token"].asString();
        const std::string scope = tokenJson["scope"].asString();
        const int expiresIn = tokenJson.get("expires_in", 2592000).asInt();
        const int refreshExpiresIn = tokenJson.get("refresh_token_expires_in", 31536000).asInt();

        const auto now = std::chrono::system_clock::now();
        const auto accessExpiresAt = now + std::chrono::seconds(expiresIn);
        const auto refreshExpiresAt = now + std::chrono::seconds(refreshExpiresIn);

        PinterestTokenModel tokenModel(accessToken, accessExpiresAt, refreshToken, refreshExpiresAt, scope);
        const std::string query = tokenModel.sqlUpsert();

        auto dbPromise = std::make_shared<std::promise<std::optional<std::string>>>();
        auto dbFuture = dbPromise->get_future();

        drogon::app().getLoop()->queueInLoop([dbPromise, query]() {
            const auto dbClient = drogon::app().getFastDbClient("default");
            if(!dbClient) {
                dbPromise->set_value("DB client not available");
                return;
            }
            dbClient->execSqlAsync(
                query,
                [dbPromise](const drogon::orm::Result &) {
                    dbPromise->set_value(std::nullopt);
                },
                [dbPromise](const drogon::orm::DrogonDbException &e) {
                    dbPromise->set_value(e.base().what());
                });
        });

        if(const auto dbError = dbFuture.get()) {
            sentryHelper(std::runtime_error(*dbError), "PinterestOAuth::callback");
            Json::Value json;
            json["error"] = "Failed to save tokens to database";
            const auto resp = HttpResponse::newHttpJsonResponse(std::move(json));
            resp->setStatusCode(HttpStatusCode::k500InternalServerError);
            (*callbackPtr)(resp);
            return;
        }

        Json::Value json;
        json["access_token_expires_at"] = formatExpiry(accessExpiresAt);
        json["refresh_token_expires_at"] = formatExpiry(refreshExpiresAt);
        json["scope"] = scope;
        json["message"] = "Tokens saved successfully";
        const auto resp = HttpResponse::newHttpJsonResponse(std::move(json));
        resp->setStatusCode(HttpStatusCode::k200OK);
        (*callbackPtr)(resp);
    }
}  // namespace

void PinterestOAuth::getOAuthUrl(const HttpRequestPtr &,
                                 std::function<void(const HttpResponsePtr &)> &&callback) const {
    const std::string state = drogon::utils::genRandomString(32);
    {
        std::lock_guard lock(stateMutex);
        pendingState = state;
    }

    const std::string oauthUri = getEnv("PINTEREST_OAUTH_URI", "https://www.pinterest.com");
    const std::string clientId = getEnv("PINTEREST_CLIENT_ID");
    const std::string redirectUri =
        getEnv("PINTEREST_REDIRECT_URI", "http://localhost:8080/admin/pinterest/oauth/callback");
    constexpr std::string_view scope = "pins:read,pins:write,user_accounts:read,boards:read,boards:write";

    const std::string url = fmt::format("{}/oauth/?consumer_id={}&redirect_uri={}&scope={}&response_type=code&state={}",
                                        oauthUri,
                                        clientId,
                                        redirectUri,
                                        scope,
                                        state);

    Json::Value json;
    json["url"] = url;
    const auto resp = HttpResponse::newHttpJsonResponse(std::move(json));
    resp->setStatusCode(HttpStatusCode::k200OK);
    callback(resp);
}

void PinterestOAuth::callback(const HttpRequestPtr &req,
                              std::function<void(const HttpResponsePtr &)> &&callback) const {
    const std::string code = req->getParameter("code");
    const std::string state = req->getParameter("state");

    // Validate state first without clearing it
    {
        std::lock_guard lock(stateMutex);
        if(state.empty() || state != pendingState) {
            Json::Value json;
            json["error"] = "Invalid or expired state parameter";
            const auto resp = HttpResponse::newHttpJsonResponse(std::move(json));
            resp->setStatusCode(HttpStatusCode::k400BadRequest);
            callback(resp);
            return;
        }
    }

    // Validate code parameter
    if(code.empty()) {
        Json::Value json;
        json["error"] = "Missing code parameter";
        const auto resp = HttpResponse::newHttpJsonResponse(std::move(json));
        resp->setStatusCode(HttpStatusCode::k400BadRequest);
        callback(resp);
        return;
    }

    // Only clear pendingState after all validations pass
    {
        std::lock_guard lock(stateMutex);
        pendingState.clear();
    }

    const auto callbackPtr = std::make_shared<std::function<void(const HttpResponsePtr &)>>(std::move(callback));

    // Run blocking CPR call off the event loop thread
    std::jthread([callbackPtr, code]() {
        exchangeTokenAndSave(callbackPtr, code);
    }).detach();
}
