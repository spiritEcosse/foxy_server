#pragma once

#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "BasketItemModel.h"

namespace api::v1 {
    class BasketItem final : public drogon::HttpController<BasketItem>, public BaseCRUD<BasketItemModel, BasketItem> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(BasketItem::getOne, "{1}", drogon::Get, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(BasketItem::updateItems, "items", drogon::Put, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(BasketItem::createItem, "", drogon::Post, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(BasketItem::getList, "", drogon::Get, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(BasketItem::deleteItem, "{1}", drogon::Delete, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(BasketItem::getOne, "admin/{1}", drogon::Get, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(BasketItem::updateItem,
                   "admin/{1}",
                   drogon::Put,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(BasketItem::createItem, "admin", drogon::Post, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(BasketItem::getList, "admin", drogon::Get, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(BasketItem::deleteItem,
                   "admin/{1}",
                   drogon::Delete,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_LIST_END
    };
}
