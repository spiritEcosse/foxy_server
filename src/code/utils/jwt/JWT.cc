#include "utils/jwt/JWT.h"
#include <fmt/core.h>
#include <cpr/cpr.h>

using namespace api::utils::jwt;
using namespace drogon;

std::expected<Json::Value, Json::Value> JWT::verifyGoogleToken(std::string token) {
    std::string url(fmt::format("https://oauth2.googleapis.com/tokeninfo?id_token={}", std::move(token)));
    const cpr::Response response = cpr::Get(cpr::Url(std::move(url)));
    Json::Value jsonResponse;
    parseJson(response, jsonResponse);
    if(response.status_code != 200)
        return std::unexpected(std::move(jsonResponse));
    return std::move(jsonResponse);
}
