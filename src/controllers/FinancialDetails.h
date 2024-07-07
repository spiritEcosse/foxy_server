#pragma once

#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "FinancialDetailsModel.h"

namespace api::v1 {
    class FinancialDetails : public drogon::HttpController<FinancialDetails>,
                             public BaseCRUD<FinancialDetailsModel, FinancialDetails> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(FinancialDetails::getList, "", drogon::Get, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(FinancialDetails::getOne, "admin/{1}", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(FinancialDetails::updateItem,
                   "admin/{1}",
                   drogon::Put,
                   drogon::Options,
                   "api::v1::filters::JwtFilter");
        METHOD_ADD(FinancialDetails::createItem, "admin", drogon::Post, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(FinancialDetails::getList, "admin", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(FinancialDetails::deleteItem,
                   "admin/{1}",
                   drogon::Delete,
                   drogon::Options,
                   "api::v1::filters::JwtFilter");
        METHOD_LIST_END
    };
}