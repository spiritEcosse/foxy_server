#pragma once

#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "BasketModel.h"

namespace api::v1 {
    class Basket : public drogon::HttpController<Basket>, public BaseCRUD<BasketModel, Basket> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(Basket::getOne, "admin/{1}", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Basket::updateItem, "admin/{1}", drogon::Put, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Basket::createItem, "admin", drogon::Post, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Basket::getList, "admin", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Basket::deleteItem, "admin/{1}", drogon::Delete, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_LIST_END
    };
}
