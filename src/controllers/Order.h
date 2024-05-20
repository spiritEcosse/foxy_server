#pragma once

#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "src/models/OrderModel.h"

namespace api::v1 {
    class Order : public drogon::HttpController<Order>, public BaseCRUD<OrderModel, Order> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(Order::getOne, "{1}", drogon::Get, drogon::Options);
        METHOD_ADD(Order::getOne, "admin/{1}", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Order::updateItem, "admin/{1}", drogon::Put, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Order::createItem, "admin", drogon::Post, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Order::getList, "admin", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Order::deleteItem, "admin/{1}", drogon::Delete, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_LIST_END
    };
}
