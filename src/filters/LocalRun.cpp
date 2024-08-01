//
// Created by ihor on 22.07.2024.
//
#include "LocalRun.h"

void api::v1::filters::LocalRun::doFilter(const drogon::HttpRequestPtr &request,
                                          drogon::FilterCallback &&fcb,
                                          drogon::FilterChainCallback &&fccb) {
    auto clientIP = request->getPeerAddr().toIp();
    if(clientIP != "127.0.0.1") {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k403Forbidden);
        return fcb(resp);
    }
    return fccb();
}
