#pragma once

#include <drogon/drogon.h>
#include <src/utils/jwt/JWT.h>
#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "src/models/ItemModel.h"

namespace api::v1 {

    class Item : public drogon::HttpController<Item>, public BaseCRUD<ItemModel, Item> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(Item::getList, "", drogon::Get, drogon::Options);
        METHOD_ADD(Item::getOne, "{1}", drogon::Get, drogon::Options);
        METHOD_ADD(Item::getListAdmin, "admin", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Item::getOne, "admin/{1}", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Item::createItem, "admin", drogon::Post, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Item::updateItem, "admin/{1}", drogon::Put, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Item::deleteItem, "admin/{1}", drogon::Delete, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_LIST_END
        void getListAdmin(const drogon::HttpRequestPtr &req,
                          std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
        void getOne(const drogon::HttpRequestPtr &req,
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                    const std::string &) const;
    };
}
