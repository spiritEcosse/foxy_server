#pragma once

#include <drogon/HttpController.h>

namespace gallery
{

class ItemController : public drogon::HttpController<ItemController>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ItemController::get, "/item/", drogon::Get, drogon::Options); // path is /item/
    ADD_METHOD_TO(ItemController::getItem, "/item/{1}/", drogon::Get, drogon::Options); // path is /item/1/

    METHOD_LIST_END
    // your declaration of processing function maybe like this:
     void get(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) const;
     void getItem(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, const std::string& ) const;
};
}
