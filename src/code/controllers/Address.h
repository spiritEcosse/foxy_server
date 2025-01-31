#pragma once

#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "AddressModel.h"

namespace api::v1 {
    class Address final : public drogon::HttpController<Address>, public BaseCRUD<AddressModel, Address> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(Address::createItem, "", drogon::Post, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Address::updateItem, "{1}", drogon::Put, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Address::getList, "", drogon::Get, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Address::getOne, "admin/{1}", drogon::Get, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Address::updateItem, "admin/{1}", drogon::Put, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Address::createItem, "admin", drogon::Post, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Address::getList, "admin", drogon::Get, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Address::deleteItem,
                   "admin/{1}",
                   drogon::Delete,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_LIST_END
    };
}
