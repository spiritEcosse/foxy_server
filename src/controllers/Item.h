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
        METHOD_ADD(Item::getList, "/", drogon::Get, drogon::Options);  // path is /item/
        METHOD_ADD(Item::getOne, "{1}/", drogon::Get, drogon::Options);  // path is /item/1/
        METHOD_ADD(Item::createItem, "/", drogon::Post, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Item::updateItem, "{1}/", drogon::Put, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Item::deleteItem, "{1}/", drogon::Delete, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_LIST_END
        [[nodiscard]] static Json::Value getJsonResponse(const drogon::orm::Result &r);
    };
}
