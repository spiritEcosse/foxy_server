#pragma once

#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "CountryModel.h"

namespace api::v1 {
    class Country : public drogon::HttpController<Country>, public BaseCRUD<CountryModel, Country> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(Country::getOne, "admin/{1}", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Country::updateItem, "admin/{1}", drogon::Put, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Country::createItem, "admin", drogon::Post, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Country::getList, "admin", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Country::deleteItem, "admin/{1}", drogon::Delete, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_LIST_END
    };
}
