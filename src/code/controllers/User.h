#pragma once

#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "UserModel.h"

namespace api::v1 {
    class User final : public drogon::HttpController<User>, public BaseCRUD<UserModel, User> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(User::getOne, "{1}", drogon::Get, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(User::deleteItem, "{1}", drogon::Delete, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(User::getOne, "admin/{1}", drogon::Get, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(User::updateItem, "admin/{1}", drogon::Put, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(User::createItem, "admin", drogon::Post, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(User::getList, "admin", drogon::Get, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(User::deleteItem, "admin/{1}", drogon::Delete, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_LIST_END
    };
}
