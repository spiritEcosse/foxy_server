#pragma once

#include <drogon/drogon.h>
#include <JWT.h>

namespace api::v1::filters {
    class JwtGoogleFilter : public drogon::HttpFilter<JwtGoogleFilter> {
    public:
        JwtGoogleFilter() = default;

        void doFilter(const drogon::HttpRequestPtr &request,
                      drogon::FilterCallback &&fcb,
                      drogon::FilterChainCallback &&fccb) override;
        static std::tuple<bool, Json::Value>
        verifyTokenAndRespond(const std::string &credentialsStr,
                              std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr);
    };
}