#include "JWT.h"
#include <fmt/core.h>
#include <cpr/cpr.h>

using namespace api::utils::jwt;
using namespace drogon;

std::tuple<HttpStatusCode, Json::Value> JWT::verifyGoogleToken(std::string token) {
    std::string url(fmt::format("https://oauth2.googleapis.com/tokeninfo?id_token={}", std::move(token)));
    const cpr::Response response = cpr::Get(cpr::Url(std::move(url)));
    Json::Value jsonResponse;
    parseJson(response, jsonResponse);
    return {static_cast<HttpStatusCode>(response.status_code), std::move(jsonResponse)};
}
