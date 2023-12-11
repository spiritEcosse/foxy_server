#pragma once

#include <drogon/HttpController.h>


namespace gallery
{
class PageController : public drogon::HttpController<PageController>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(PageController::getItem, "/page/{1}/", drogon::Get, drogon::Options); // path is /item/
    METHOD_LIST_END
    void getItem(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, std::string ) const;
};
}
