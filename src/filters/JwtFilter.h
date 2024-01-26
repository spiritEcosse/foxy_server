#pragma once

#include <drogon/drogon.h>
#include <src/utils/jwt/JWT.h>

namespace api::v1::filters {
class JwtFilter : public drogon::HttpFilter<JwtFilter> {
    public:
        JwtFilter() = default;

        void doFilter(const drogon::HttpRequestPtr &request, drogon::FilterCallback &&fcb, drogon::FilterChainCallback &&fccb) override;
    };
}