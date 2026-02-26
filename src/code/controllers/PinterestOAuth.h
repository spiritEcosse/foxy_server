#pragma once

#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "models/PinterestTokenModel.h"
#include <mutex>
#include <string>

namespace api::v1 {
    class PinterestOAuth final : public drogon::HttpController<PinterestOAuth>,
                                 public BaseCRUD<PinterestTokenModel, PinterestOAuth> {
        static inline std::mutex stateMutex{};
        static inline std::string pendingState{};

    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(PinterestOAuth::getList,
                   "admin/pinterest/oauth",
                   drogon::Get,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(PinterestOAuth::getOne,
                   "admin/pinterest/oauth/{1}",
                   drogon::Get,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(PinterestOAuth::getOAuthUrl,
                   "admin/pinterest/oauth/url",
                   drogon::Get,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(PinterestOAuth::callback, "admin/pinterest/oauth/callback", drogon::Get, drogon::Options);
        METHOD_LIST_END

        void getOAuthUrl(const drogon::HttpRequestPtr &req,
                         std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;

        void callback(const drogon::HttpRequestPtr &req,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
    };
}
