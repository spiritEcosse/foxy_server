#pragma once

#include <drogon/drogon.h>
#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "models/CollectionModel.h"

namespace api::v1 {

    class Collection final : public drogon::HttpController<Collection>, public BaseCRUD<CollectionModel, Collection> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(Collection::getList, "", drogon::Get, drogon::Options);
        METHOD_ADD(Collection::getOne, "{1}", drogon::Get, drogon::Options);
        METHOD_ADD(Collection::getListAdmin,
                   "admin",
                   drogon::Get,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Collection::getOneAdmin,
                   "admin/{1}",
                   drogon::Get,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Collection::createItem, "admin", drogon::Post, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Collection::updateItem,
                   "admin/{1}",
                   drogon::Put,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(Collection::deleteItem,
                   "admin/{1}",
                   drogon::Delete,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_LIST_END

        // Public lookup is slug-only: a numeric / non-slug id must 404, never resolve by id.
        void getOne(const drogon::HttpRequestPtr &req,
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                    std::string &&stringId) const override;

        void getListAdmin(const drogon::HttpRequestPtr &req,
                          std::function<void(const drogon::HttpResponsePtr &)> &&callback) const;
        void getOneAdmin(const drogon::HttpRequestPtr &req,
                         std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                         std::string &&stringId) const;
    };
}
