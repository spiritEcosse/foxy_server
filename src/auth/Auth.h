#pragma once

#include <drogon/drogon.h>
#include <drogon/HttpController.h>

namespace api::v1 {
    class Auth : public drogon::HttpController<Auth> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(Auth::getToken, "/login", drogon::Post, drogon::Options);
        METHOD_ADD(Auth::verifyToken, "/verify", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_LIST_END

        void getToken(const drogon::HttpRequestPtr &request,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
        static void verifyToken(const drogon::HttpRequestPtr &request,
                                std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    };
}