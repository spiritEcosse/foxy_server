#pragma once

#include "drogon/HttpController.h"
#include "BaseCRUD.h"
#include "src/models/PageModel.h"

namespace api::v1
{
class Page: public drogon::HttpController<Page>, public BaseCRUD<PageModel, Page>
{
public:
    METHOD_LIST_BEGIN
        METHOD_ADD(Page::getOne, "{1}", drogon::Get, drogon::Options);
        METHOD_ADD(Page::getOne, "admin/{1}", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Page::updateItem, "admin/{1}", drogon::Put, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Page::createItem, "admin", drogon::Post, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Page::getList, "admin", drogon::Get, drogon::Options, "api::v1::filters::JwtFilter");
        METHOD_ADD(Page::deleteItem, "admin/{1}", drogon::Delete, drogon::Options, "api::v1::filters::JwtFilter");
    METHOD_LIST_END

    void getOne(const drogon::HttpRequestPtr &req,
                std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                const std::string &) const;
};
}
