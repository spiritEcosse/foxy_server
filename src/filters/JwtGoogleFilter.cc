#include "JwtGoogleFilter.h"

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

        auto res = HttpResponse::newHttpJsonResponse(resultJson);
        res->setStatusCode(k401Unauthorized);

        // Return the response and let's tell this endpoint request was cancelled
        return fcb(res);
    }

    // Remove the string "Bearer " on token and decode it
    auto [statusCode, jsonResponse] = JWT::verifyGoogleToken(token.substr(7));
    if(statusCode != drogon::k200OK) {
        auto res = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
        if(statusCode >= 500) {
            statusCode = drogon::k424FailedDependency;
        }
        res->setStatusCode(statusCode);
        return fcb(res);
    }
    return fccb();
}
