#pragma once

#include <drogon/drogon.h>

namespace api::v1::filters {
    class LocalRun final : public drogon::HttpFilter<LocalRun> {
    public:
        LocalRun() = default;

        void doFilter(const drogon::HttpRequestPtr &request,
                      drogon::FilterCallback &&fcb,
                      drogon::FilterChainCallback &&fccb) override;
    };
}
