#pragma once

#include "drogon/HttpController.h"
#include "MediaModel.h"

namespace api::v1 {
    class Media final : public drogon::HttpController<Media>, public BaseCRUD<MediaModel, Media> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(Media::getOne, "{1}", drogon::Get, drogon::Options);
        METHOD_ADD(Media::createItem, "admin", drogon::Post, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Media::updateItem, "admin/{1}", drogon::Put, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Media::updateItems,
                   "admin/items",
                   drogon::Put,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Media::createItems,
                   "admin/items",
                   drogon::Post,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Media::getList, "admin/", drogon::Get, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Media::deleteItem,
                   "admin/{1}",
                   drogon::Delete,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Media::deleteItems,
                   "admin/items",
                   drogon::Delete,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_LIST_END
    };
}
