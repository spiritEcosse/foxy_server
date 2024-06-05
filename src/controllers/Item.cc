#include "Item.h"
#include "src/utils/request/Request.h"
#include "src/orm/QuerySet.h"
#include "src/models/MediaModel.h"
#include "src/utils/env.h"
#include <fmt/core.h>

using namespace api::v1;
using namespace drogon::orm;

void Item::getListAdmin(const drogon::HttpRequestPtr &req,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    int page = getInt(req->getParameter("page"), 1);
    int limit = getInt(req->getParameter("limit"), 25);

    std::string app_cloud_name;
    getenv("APP_CLOUD_NAME", app_cloud_name);

    QuerySet qsCount = ItemModel().qsCount();
    QuerySet qsPage = ItemModel().qsPage(page, limit);

    QuerySet qs(ItemModel::tableName, limit, "data");
    auto mediaSort = MediaModel::Field::sort.getFullFieldName();
    auto orderByItemField = BaseModel<ItemModel>::Field::updatedAt;
    auto itemID = BaseModel<ItemModel>::Field::id;
    auto mediaItemID = MediaModel::Field::itemId.getFullFieldName();
    qs.distinct(orderByItemField, itemID)
        .left_join(MediaModel())
        .filter(mediaSort, std::string("NULL"), false, std::string("IS"), std::string("OR"))
        .filter(mediaSort,
                std::string(fmt::format("(SELECT MIN({}) FROM {} WHERE {} = {})",
                                        mediaSort,
                                        MediaModel::tableName,
                                        BaseModel<ItemModel>::Field::id.getFullFieldName(),
                                        mediaItemID)),
                false)
        .order_by(std::make_pair(orderByItemField, false), std::make_pair(itemID, false))
        .only(ItemModel().allSetFields())
        .functions(Function(fmt::format("format_src(media.src, '{}') as src", app_cloud_name)))
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit));
    executeSqlQuery(callbackPtr, QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs)));
}

void Item::getOne(const drogon::HttpRequestPtr &req,
                  std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                  const std::string &stringId) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    bool isInt = canBeInt(stringId);
    if(auto resp = check404(req, !isInt && ItemModel::Field::slug.empty())) {
        (*callbackPtr)(resp);
        return;
    }

    std::string filterKey =
        isInt ? BaseModel<ItemModel>::Field::id.getFullFieldName() : ItemModel::Field::slug.getFullFieldName();
    std::string query = ItemModel().sqlSelectOne(filterKey, stringId, {});

    executeSqlQuery(callbackPtr, query);
}
