#pragma once

#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "SocialMediaModel.h"

namespace api::v1 {
    class SocialMedia : public drogon::HttpController<SocialMedia>, public BaseCRUD<SocialMediaModel, SocialMedia> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(SocialMedia::publish, "admin/publish", drogon::Post, drogon::Options);
        METHOD_ADD(SocialMedia::getOne, "admin/{1}", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(SocialMedia::updateItem, "admin/{1}", drogon::Put, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(SocialMedia::createItem, "admin", drogon::Post, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(SocialMedia::getList, "admin", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(SocialMedia::deleteItem,
                   "admin/{1}",
                   drogon::Delete,
                   drogon::Options,
                   "api::v1::filters::JwtFilter");
        METHOD_LIST_END

        void publish(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        void handleSqlResultPublish(const drogon::orm::Result &r);
        void handleRow(const auto &row);
    };
}
