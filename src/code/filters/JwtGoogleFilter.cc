#include "filters/JwtGoogleFilter.h"
#include "utils/config.h"
#include <utils/jwt/JWT.h>

using namespace drogon;
using namespace api::v1::filters;
using namespace api::utils::jwt;

void JwtGoogleFilter::doFilter(const HttpRequestPtr &request, FilterCallback &&fcb, FilterChainCallback &&fccb) {
    if(request->getMethod() == HttpMethod::Options)
        return fccb();

    const std::string &token = request->getHeader("Authorization");

    if(token.length() < 7) {
        Json::Value resultJson;
        resultJson["error"] = "No header authentication!";
        resultJson["status"] = 0;

        const auto res = HttpResponse::newHttpJsonResponse(resultJson);

        if(const auto origin = request->getHeader("Origin");
           origin == getEnv("FOXY_CLIENT", "") || origin == getEnv("FOXY_ADMIN", "")) {
            res->addHeader("Access-Control-Allow-Origin", origin);
        }
        res->setStatusCode(k401Unauthorized);

        return fcb(res);
    }

    if(auto tokenResult = JWT::verifyGoogleToken(token.substr(7)); !tokenResult) {
        const auto res = HttpResponse::newHttpJsonResponse(std::move(tokenResult.error()));
        res->setStatusCode(drogon::k401Unauthorized);
        res->setContentTypeCode(CT_APPLICATION_JSON);
        if(const auto origin = request->getHeader("Origin");
           origin == getEnv("FOXY_CLIENT", "") || origin == getEnv("FOXY_ADMIN", "")) {
            res->addHeader("Access-Control-Allow-Origin", origin);
        }

        return fcb(res);
    }
    return fccb();
}

std::expected<Json::Value, std::monostate>
JwtGoogleFilter::verifyTokenAndRespond(const std::string &credentialsStr,
                                       std::shared_ptr<std::function<void(const HttpResponsePtr &)>> callbackPtr) {
    auto tokenResult = JWT::verifyGoogleToken(credentialsStr);
    if(!tokenResult) {
        const auto res = HttpResponse::newHttpJsonResponse(std::move(tokenResult.error()));
        res->setStatusCode(drogon::k401Unauthorized);
        res->setContentTypeCode(ContentType::CT_APPLICATION_JSON);
        (*callbackPtr)(res);
        return std::unexpected(std::monostate{});
    }
    return std::move(*tokenResult);
}
