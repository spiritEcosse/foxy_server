#include "Item.h"
#include "src/utils/request/Request.h"
#include "src/orm/QuerySet.h"
#include "src/models/MediaModel.h"
#include "src/utils/env.h"
#include <fmt/core.h>

using namespace api::v1;
using namespace drogon::orm;

Json::Value Item::getJsonResponse(const Result &r) {
    if (r[0].size() != 2) {
        return BaseCRUD::getJsonResponse(r);
    }
    auto jsonResponse = r[0][1].as<Json::Value>();
    jsonResponse["media"] = r[0][0].as<Json::Value>();
    return jsonResponse;
}

void Item::getListAdmin(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    int page = getInt(req->getParameter("page"), 1);
    int limit = getInt(req->getParameter("limit"), 25);

    std::string app_cloud_name;
    getenv("APP_CLOUD_NAME", app_cloud_name);
    QuerySet qs(ItemModel::tableName, false, limit, page, true);
    qs.distinct(ItemModel::tableName + "." + ItemModel::orderBy, ItemModel::tableName + "." + ItemModel::Field::id)
        .left_join(MediaModel::tableName, ItemModel::tableName + "." + ItemModel::Field::id + " = " + MediaModel::tableName + "." + MediaModel::Field::itemId)
        .order_by(std::make_pair(ItemModel::tableName + "." + ItemModel::orderBy, false), std::make_pair(ItemModel::tableName + "." + ItemModel::Field::id, false))
        .only({ItemModel::fullFieldsWithTableToString(), fmt::format("format_src(media.src, '{}') as src", app_cloud_name)});
    executeSqlQuery(callbackPtr, qs.buildSelect(),
                    [this](const drogon::orm::Result &r, std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> _callbackPtr) {
                        this->handleSqlResultList(r, _callbackPtr);
                    });
}
