#pragma once

#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "ShippingRateModel.h"

namespace api::v1 {
    class ShippingRate : public drogon::HttpController<ShippingRate>, public BaseCRUD<ShippingRateModel, ShippingRate> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(ShippingRate::getShippingRateByItem, "item/{}", drogon::Get, drogon::Options);
        METHOD_ADD(ShippingRate::getOne, "admin/{1}", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(ShippingRate::updateItem, "admin/{1}", drogon::Put, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(ShippingRate::createItem, "admin", drogon::Post, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(ShippingRate::getList, "admin", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(ShippingRate::deleteItem,
                   "admin/{1}",
                   drogon::Delete,
                   drogon::Options,
                   "api::v1::filters::JwtFilter");
        METHOD_LIST_END
        virtual void getShippingRateByItem(const drogon::HttpRequestPtr &req,
                                           std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                           const std::string &) const;
    };
}
