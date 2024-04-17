#include "Item.h"
#include "src/utils/request/Request.h"
#include "src/orm/QuerySet.h"
#include "src/models/MediaModel.h"
#include "src/utils/env.h"
#include <fmt/core.h>

using namespace api::v1;
using namespace drogon::orm;

Json::Value Item::getJsonResponse(const Result &r) {
    if(r[0].size() != 2) {
        return BaseCRUD::getJsonResponse(r);
    }
    auto jsonResponse = r[0][1].as<Json::Value>();
    jsonResponse["media"] = r[0][0].as<Json::Value>();
    return jsonResponse;
}

void Item::getListAdmin(const drogon::HttpRequestPtr &req,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    int page = getInt(req->getParameter("page"), 1);
    int limit = getInt(req->getParameter("limit"), 25);

    std::string app_cloud_name;
    getenv("APP_CLOUD_NAME", app_cloud_name);
    QuerySet qs(ItemModel::tableName, false, limit, page, true);
    auto mediaSort = fmt::format("{}.{}", MediaModel::tableName, MediaModel::Field::sort);
    auto orderByItemField = fmt::format("{}.{}", ItemModel::tableName, ItemModel::orderBy);
    auto itemID = fmt::format("{}.{}", ItemModel::tableName, ItemModel::Field::id);
    auto mediaItemID = fmt::format("{}.{}", MediaModel::tableName, MediaModel::Field::itemId);
    qs.distinct(orderByItemField, itemID)
        .left_join(MediaModel::tableName,
                   ItemModel::tableName + "." + ItemModel::Field::id + " = " + MediaModel::tableName + "." +
                       MediaModel::Field::itemId)
        .or_filter(std::make_tuple(mediaSort, "IS", "NULL", false))
        .or_filter(mediaSort,
                   std::string(fmt::format("(SELECT MIN({}) FROM {} WHERE {} = {})",
                                           mediaSort,
                                           MediaModel::tableName,
                                           itemID,
                                           mediaItemID)),
                   false)
        .order_by(std::make_pair(orderByItemField, false), std::make_pair(itemID, false))
        .only({ItemModel::fullFieldsWithTableToString(),
               fmt::format("format_src(media.src, '{}') as src", app_cloud_name)});
    executeSqlQuery(callbackPtr,
                    qs.buildSelect(),
                    [this](const drogon::orm::Result &r,
                           std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> _callbackPtr) {
                        this->handleSqlResultList(r, _callbackPtr);
                    });
}
