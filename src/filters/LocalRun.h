//
// Created by ihor on 22.07.2024.
//

#ifndef LOCALRUN_H
#define LOCALRUN_H

#include <drogon/drogon.h>

namespace api::v1::filters {
    class LocalRun : public drogon::HttpFilter<LocalRun> {
    public:
        LocalRun() = default;

        void doFilter(const drogon::HttpRequestPtr &request,
                      drogon::FilterCallback &&fcb,
                      drogon::FilterChainCallback &&fccb) override;
    };
}
#endif  //LOCALRUN_H
