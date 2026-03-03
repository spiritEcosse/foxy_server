#include "filters/LocalRun.h"
#include <json/json.h>

void api::v1::filters::LocalRun::doFilter(const drogon::HttpRequestPtr &request,
                                          drogon::FilterCallback &&fcb,
                                          drogon::FilterChainCallback &&fccb) {
    if(const auto clientIP = request->getPeerAddr().toIp();
       clientIP != "127.0.0.1" && clientIP != "::1") {
        Json::Value json;
        json["error"] = "Forbidden";
        const auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(json));
        resp->setStatusCode(drogon::HttpStatusCode::k403Forbidden);
        return fcb(resp);
    }
    return fccb();
}
