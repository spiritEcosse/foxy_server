#pragma once

#include "drogon/HttpController.h"

namespace api::v1 {
    class Page : public drogon::HttpController<Page> {
    public:
        METHOD_LIST_BEGIN
        ADD_METHOD_TO(Page::getItem, "/page/{1}/", drogon::Get, drogon::Options);  // path is /item/
        METHOD_ADD(Page::createItem, "", drogon::Post, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Page::updateItem, "{1}", drogon::Put, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_LIST_END
        void getItem(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                     std::string) const;
        void createItem(const drogon::HttpRequestPtr &req,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
        void updateItem(const drogon::HttpRequestPtr &req,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                        const std::string &);
    };
}
