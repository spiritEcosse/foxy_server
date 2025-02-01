#include "Page.h"
#include "Request.h"
#include "MediaModel.h"
#include "env.h"
#include <fmt/core.h>

using namespace api::v1;
using namespace drogon::orm;

void Page::getOne(const drogon::HttpRequestPtr &req,
                  std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                  const std::string &stringId) const {
    const auto callbackPtr =
        std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    const bool isInt = canBeInt(stringId);
    if(const auto resp = check404(req, !isInt && PageModel::Field::slug.empty())) {
        (*callbackPtr)(resp);
        return;
    }

    const auto &filterKey = isInt ? &BaseModel<PageModel>::Field::id : &PageModel::Field::slug;
    const std::string query = PageModel().sqlSelectOne(filterKey, stringId, {});
    executeSqlQuery(callbackPtr, query);
}
