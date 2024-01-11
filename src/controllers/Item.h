#pragma once

#include <drogon/drogon.h>
#include <src/utils/jwt/JWT.h>
#include "drogon/HttpController.h"

namespace api::v1 {

    class Item : public drogon::HttpController<Item> {
    public:
        METHOD_LIST_BEGIN
        ADD_METHOD_TO(Item::get, "/item/", drogon::Get, drogon::Options);  // path is /item/
        ADD_METHOD_TO(Item::getItem, "/item/{1}/", drogon::Get, drogon::Options);  // path is /item/1/
        METHOD_ADD(Item::createItem, "", drogon::Post, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Item::updateItem, "{1}", drogon::Put, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_LIST_END
        void get(const drogon::HttpRequestPtr &req,
                 std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
        void getItem(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                     const std::string &) const;
        void createItem(const drogon::HttpRequestPtr &req,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
        void updateItem(const drogon::HttpRequestPtr &req,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                        const std::string &);
    };
}
