#pragma once

#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "src/models/ShippingProfileModel.h"

namespace api::v1 {
class ShippingProfile : public drogon::HttpController<ShippingProfile>, public BaseCRUD<ShippingProfileModel, ShippingProfile> {
public:
    METHOD_LIST_BEGIN
        METHOD_ADD(ShippingProfile::getOne, "admin/{1}", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(ShippingProfile::updateItem, "admin/{1}", drogon::Put, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(ShippingProfile::createItem, "admin", drogon::Post, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(ShippingProfile::getList, "admin", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(ShippingProfile::deleteItem, "admin/{1}", drogon::Delete, drogon::Options, "api::v1::filters::JwtFilter");
    METHOD_LIST_END
};
}
