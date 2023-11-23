#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

namespace gallery
{

class ItemController : public drogon::HttpController<ItemController>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ItemController::get, "/item/", Get, Options); // path is /item/
    ADD_METHOD_TO(ItemController::getItem, "/item/{1}/", Get, Options); // path is /item/

    METHOD_LIST_END
    // your declaration of processing function maybe like this:
     void get(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) const;
     void getItem(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, int ) const;
};
}
