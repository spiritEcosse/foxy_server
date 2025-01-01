#pragma once

#include <drogon/drogon.h>
#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "TagModel.h"

namespace api::v1 {

    class Tag final : public drogon::HttpController<Tag>, public BaseCRUD<TagModel, Tag> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(Tag::getList, "admin", drogon::Get, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Tag::updateItems, "admin/items", drogon::Put, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Tag::createItems, "admin/items", drogon::Post, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Tag::deleteItems,
                   "admin/items",
                   drogon::Delete,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_LIST_END
    };
}
