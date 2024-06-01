#include "Page.h"
#include "src/utils/request/Request.h"
#include "src/models/MediaModel.h"
#include "src/utils/env.h"
#include <fmt/core.h>

using namespace api::v1;
using namespace drogon::orm;

void Page::getOne(const drogon::HttpRequestPtr &req,
                  std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                  const std::string &stringId) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    bool isInt = canBeInt(stringId);
    if(auto resp = check404(req, !isInt && PageModel::Field::slug.empty())) {
        (*callbackPtr)(resp);
        return;
    }

    std::string filterKey = isInt ? PageModel::Field::id.getFullFieldName() : PageModel::Field::slug.getFullFieldName();
    std::string query = PageModel().sqlSelectOne(filterKey, stringId, {});

    executeSqlQuery(callbackPtr, query);
}
