#pragma once

#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "ReviewModel.h"

namespace api::v1 {
    class Review : public drogon::HttpController<Review>, public BaseCRUD<ReviewModel, Review> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(Review::getOne, "admin/{1}", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Review::updateItem, "admin/{1}", drogon::Put, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Review::createItem, "admin", drogon::Post, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Review::getList, "admin", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Review::deleteItem, "admin/{1}", drogon::Delete, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_LIST_END
    };
}
