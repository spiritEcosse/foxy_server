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
    auto qsPage = ItemModel::qsPage(page, limit);

    QuerySet<ItemModel> qs(limit, "data");
    const auto &orderByItemField = &BaseModel<ItemModel>::Field::updatedAt;
    const auto &itemID = &BaseModel<ItemModel>::Field::id;
    qs.left_join<MediaModel>(MediaModel::qsMediaMinSort(),
                             "image_media",
                             fmt::format("AND {0}.type = 'image' AND {0}.row_num = 1", MediaModel::tableName))
        .order_by(orderByItemField, false)
        .order_by(itemID, false)
        .only(ItemModel::allSetFields())
        .functions(Function(fmt::format("format_src(image_media.src, '{}') as src", APP_CLOUD_NAME)))
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit));

    executeSqlQuery(callbackPtr,
                    BuildComplexQueries::buildQuery(ItemModel::qsCount(), std::move(qsPage), std::move(qs)));
}

void Item::getOne(const drogon::HttpRequestPtr &req,
                  std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                  std::string &&stringId) const {
    const auto callbackPtr =
        std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    const bool isInt = canBeInt(stringId);
    if(const auto resp = check404(req, !isInt && ItemModel::Field::slug.empty())) {
        (*callbackPtr)(resp);
        return;
    }

    const auto &filterKey = isInt ? &BaseModel<ItemModel>::Field::id : &ItemModel::Field::slug;
    const std::string query = ItemModel().sqlSelectOne(filterKey, std::move(stringId), {});

    executeSqlQuery(callbackPtr, query);
}

void Item::getOneAdmin(const drogon::HttpRequestPtr &req,
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                       std::string &&stringId) const {
    const bool isInt = canBeInt(stringId);
    const auto callbackPtr =
        std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    if(const auto resp = check404(req, !isInt)) {
        (*callbackPtr)(resp);
        return;
    }

    QuerySet<MediaModel> qsMedia(0, MediaModel::tableName, false);
    qsMedia.filter(&MediaModel::Field::itemId, &BaseModel<ItemModel>::Field::id)
        .functions(Function(
            fmt::format("json_agg(json_build_object({}) ORDER BY media.sort ASC)", MediaModel().fieldsJsonObject())));

    QuerySet<TagModel> qsTags(0, TagModel::tableName, false);
    qsTags.filter(&TagModel::Field::itemId, &BaseModel<ItemModel>::Field::id)
        .functions(Function(
            fmt::format("json_agg(json_build_object({}) ORDER BY updated_at DESC)", TagModel().fieldsJsonObject())));

    QuerySet<ItemModel> qsItem(ItemModel::tableName, true, true);
    qsItem.filter(&BaseModel<ItemModel>::Field::id, std::move(stringId))
        .jsonFields(addExtraQuotes(ItemModel().fieldsJsonObject()))
        .functions(Function(addExtraQuotes(fmt::format(R"(
        'media', COALESCE(({0}), '[]'::json),
        'tags', COALESCE(({1}), '[]'::json)
    )",
                                                       qsMedia.buildSelect(),
                                                       qsTags.buildSelect()))));

    executeSqlQuery(callbackPtr, qsItem.buildSelectOne());
}