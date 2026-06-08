#pragma once

#include <drogon/drogon.h>
#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "models/CollectionItemModel.h"

namespace api::v1 {

    class CollectionItem final : public drogon::HttpController<CollectionItem>,
                                 public BaseCRUD<CollectionItemModel, CollectionItem> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(CollectionItem::getList, "admin", drogon::Get, drogon::Options, "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(CollectionItem::createItems,
                   "admin/items",
                   drogon::Post,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(CollectionItem::updateItems,
                   "admin/items",
                   drogon::Put,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_ADD(CollectionItem::deleteItems,
                   "admin/items",
                   drogon::Delete,
                   drogon::Options,
                   "api::v1::filters::JwtGoogleFilter");
        METHOD_LIST_END
    };
}
