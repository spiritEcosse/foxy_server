#include "Item.h"
#include "Request.h"
#include "QuerySet.h"
#include "MediaModel.h"
#include "TagModel.h"
#include "env.h"
#include <fmt/core.h>

using namespace api::v1;
using namespace drogon::orm;

void Item::getListAdmin(const drogon::HttpRequestPtr &req,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    const auto callbackPtr =
        std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    const int page = getInt(req->getParameter("page"), 1);
    int limit = getInt(req->getParameter("limit"), 25);

    QuerySet qsCount = ItemModel::qsCount();
    QuerySet qsPage = ItemModel::qsPage(page, limit);

    QuerySet qs(ItemModel::tableName, limit, "data");
    auto mediaSort = MediaModel::Field::sort.getFullFieldName();
    const auto &orderByItemField = &BaseModel<ItemModel>::Field::updatedAt;
    const auto &itemID = &BaseModel<ItemModel>::Field::id;
    auto mediaItemID = MediaModel::Field::itemId.getFullFieldName();
    qs.distinct(orderByItemField, itemID)
        .left_join(MediaModel())
        // .filter(mediaSort, std::string("NULL"), false, std::string("IS"), std::string("OR"))
        // .filter(mediaSort,
        //         std::string(fmt::format("(SELECT MIN({}) FROM {} WHERE {} = {})",
        //                                 mediaSort,
        //                                 MediaModel::tableName,
        //                                 BaseModel<ItemModel>::Field::id.getFullFieldName(),
        //                                 mediaItemID)),
        //         false)
        .order_by(std::make_pair(&orderByItemField, false), std::make_pair(&itemID, false))
        .only(ItemModel::allSetFields())
        .functions(Function(fmt::format("format_src(media.src, '{}') as src", APP_CLOUD_NAME)))
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit));
    executeSqlQuery(callbackPtr, QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs)));
}

void Item::getOne(const drogon::HttpRequestPtr &req,
                  std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                  const std::string &stringId) const {
    const auto callbackPtr =
        std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    const bool isInt = canBeInt(stringId);
    if(const auto resp = check404(req, !isInt && ItemModel::Field::slug.empty())) {
        (*callbackPtr)(resp);
        return;
    }

    const std::string filterKey =
        isInt ? BaseModel<ItemModel>::Field::id.getFullFieldName() : ItemModel::Field::slug.getFullFieldName();
    const std::string query = ItemModel().sqlSelectOne(filterKey, stringId, {});

    executeSqlQuery(callbackPtr, query);
}

void Item::getOneAdmin(const drogon::HttpRequestPtr &req,
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                       const std::string &stringId) const {
    const bool isInt = canBeInt(stringId);
    const auto callbackPtr =
        std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    if(const auto resp = check404(req, !isInt)) {
        (*callbackPtr)(resp);
        return;
    }

    // Media subquery with JSON aggregation
    QuerySet qsMedia(MediaModel::tableName, 0, MediaModel::tableName, false);
    qsMedia
        .filter(MediaModel::Field::itemId.getFullFieldName(), BaseModel<ItemModel>::Field::id.getFullFieldName(), false)
        .functions(Function(
            fmt::format("json_agg(json_build_object({}) ORDER BY media.sort ASC)", MediaModel().fieldsJsonObject())));

    // Tags subquery with JSON aggregation
    QuerySet qsTags(TagModel::tableName, 0, TagModel::tableName, false);
    qsTags.filter(TagModel::Field::itemId.getFullFieldName(), BaseModel<ItemModel>::Field::id.getFullFieldName(), false)
        .functions(Function(
            fmt::format("json_agg(json_build_object({}) ORDER BY updated_at DESC)", TagModel().fieldsJsonObject())));

    // Main item query with both media and tags as JSON arrays
    QuerySet qsItem(ItemModel::tableName, ItemModel::tableName, true, true);
    qsItem.filter(BaseModel<ItemModel>::Field::id.getFullFieldName(), stringId)
        .jsonFields(addExtraQuotes(ItemModel().fieldsJsonObject()))
        .functions(Function(fmt::format(R"(
        'media', COALESCE(({0}), '[]'::json),
        'tags', COALESCE(({1}), '[]'::json)
    )",
                                        qsMedia.buildSelect(),
                                        qsTags.buildSelect())));

    executeSqlQuery(callbackPtr, qsItem.buildSelectOne());
}