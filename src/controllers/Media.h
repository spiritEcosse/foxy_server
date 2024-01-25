//
// Created by ihor on 20.01.2024.
//

#ifndef MEDIA_H
#define MEDIA_H

#include "drogon/HttpController.h"
#include "src/models/MediaModel.h"

namespace api::v1 {
    class Media : public drogon::HttpController<Media>, public BaseCRUD<MediaModel, Media> {
    public:
        METHOD_LIST_BEGIN
        METHOD_ADD(Media::getOne, "/{1}", drogon::Get, drogon::Options);
        METHOD_ADD(Media::updateItem, "/{1}", drogon::Put, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Media::createItem, "", drogon::Post, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Media::createItems, "items/", drogon::Post, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Media::getList, "", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Media::deleteItem, "{1}", drogon::Delete, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_LIST_END
    };
}

#endif  //MEDIA_H
