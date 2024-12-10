#include "LocalRun.h"

void api::v1::filters::LocalRun::doFilter(const drogon::HttpRequestPtr &request,
                                          drogon::FilterCallback &&fcb,
                                          drogon::FilterChainCallback &&fccb) {
    if(const auto clientIP = request->getPeerAddr().toIp(); clientIP != "127.0.0.1") {
        const auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k403Forbidden);
        return fcb(resp);
    }
    return fccb();
}
