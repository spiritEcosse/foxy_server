#pragma once

#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "src/models/BasketItemModel.h"

namespace api::v1 {
    class BasketItem : public drogon::HttpController<BasketItem>, public BaseCRUD<BasketItemModel, BasketItem> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(BasketItem::getOne, "{1}", drogon::Get, drogon::Options);
        METHOD_ADD(BasketItem::getOne, "admin/{1}", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(BasketItem::updateItem, "admin/{1}", drogon::Put, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(BasketItem::createItem, "admin", drogon::Post, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(BasketItem::getList, "admin", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(BasketItem::deleteItem, "admin/{1}", drogon::Delete, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_LIST_END
    };
}
