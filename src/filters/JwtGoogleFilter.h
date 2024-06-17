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
    };
}