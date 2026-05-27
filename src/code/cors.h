#pragma once

#include "drogon/drogon.h"
#include "utils/config.h"

namespace cors {

inline bool isAllowedOrigin(const std::string &origin) {
    return !origin.empty() &&
           (origin == api::v1::getEnv("FOXY_CLIENT", "") || origin == api::v1::getEnv("FOXY_ADMIN", ""));
}

inline void attachCorsHeaders(const drogon::HttpRequestPtr &req, const drogon::HttpResponsePtr &resp) {
    const auto origin = req->getHeader("Origin");
    if(isAllowedOrigin(origin))
        resp->addHeader("Access-Control-Allow-Origin", origin);
}

inline void registerCorsMiddleware() {
    drogon::app().registerPostHandlingAdvice(&attachCorsHeaders);

    drogon::app().setCustomErrorHandler([](drogon::HttpStatusCode code, const drogon::HttpRequestPtr &req) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(code);
        attachCorsHeaders(req, resp);
        return resp;
    });

    drogon::app().registerSyncAdvice([](const drogon::HttpRequestPtr &req) -> drogon::HttpResponsePtr {
        if(req->getMethod() != drogon::HttpMethod::Options)
            return nullptr;
        const auto origin = req->getHeader("Origin");
        if(!isAllowedOrigin(origin))
            return nullptr;
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k204NoContent);
        resp->addHeader("Access-Control-Allow-Origin", origin);
        resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, PATCH, DELETE, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Authorization, Content-Type");
        resp->addHeader("Access-Control-Max-Age", "86400");
        return resp;
    });
}

}  // namespace cors
