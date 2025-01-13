#include "JwtGoogleFilter.h"
#include "env.h"
#include <JWT.h>

using namespace drogon;
using namespace api::v1::filters;
using namespace api::utils::jwt;

void JwtGoogleFilter::doFilter(const HttpRequestPtr &request, FilterCallback &&fcb, FilterChainCallback &&fccb) {
    // Skip the verification on method Options
    if(request->getMethod() == HttpMethod::Options)
        return fccb();

    const std::string &token = request->getHeader("Authorization");

    // If the authorization header is empty or if the length is lower than 7 characters, means "Bearer " is not included on authorization header string.
    if(token.length() < 7) {
        Json::Value resultJson;
        resultJson["error"] = "No header authentication!";
        resultJson["status"] = 0;

        const auto res = HttpResponse::newHttpJsonResponse(resultJson);

        if(const auto origin = request->getHeader("Origin"); origin == FOXY_CLIENT || origin == FOXY_ADMIN) {
            res->addHeader("Access-Control-Allow-Origin", origin);
        }
        res->setStatusCode(k401Unauthorized);

        // Return the response and let's tell this endpoint request was cancelled
        return fcb(res);
    }

    // Remove the string "Bearer " on token and decode it
    if(auto [statusCode, jsonResponse] = JWT::verifyGoogleToken(token.substr(7)); statusCode != k200OK) {
        const auto res = HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
        res->setStatusCode(drogon::k401Unauthorized);
        res->setContentTypeCode(CT_APPLICATION_JSON);
        if(const auto origin = request->getHeader("Origin"); origin == FOXY_CLIENT || origin == FOXY_ADMIN) {
            res->addHeader("Access-Control-Allow-Origin", origin);
        }

        return fcb(res);
    }
    return fccb();
}

std::tuple<bool, Json::Value>
JwtGoogleFilter::verifyTokenAndRespond(const std::string &credentialsStr,
                                       std::shared_ptr<std::function<void(const HttpResponsePtr &)>> callbackPtr) {
    auto [statusCode, jsonResponse] = JWT::verifyGoogleToken(credentialsStr);
    if(statusCode != k200OK) {
        const auto res = HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
        res->setStatusCode(drogon::k401Unauthorized);
        res->setContentTypeCode(ContentType::CT_APPLICATION_JSON);
        (*callbackPtr)(res);
        return {false, jsonResponse};
    }
    return {true, jsonResponse};  // Token is valid
}
