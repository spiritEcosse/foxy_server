#pragma once

#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "src/models/UserModel.h"

namespace api::v1 {
    class User : public drogon::HttpController<User>, public BaseCRUD<UserModel, User> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(User::getOne, "/{1}/", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(User::updateItem, "/{1}/", drogon::Put, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(User::createItem, "/", drogon::Post, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(User::getList, "/", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(User::deleteItem, "{1}/", drogon::Delete, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_LIST_END
    };
}
