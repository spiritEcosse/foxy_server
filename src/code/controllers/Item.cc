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
    const auto &orderByItemField = std::cref(BaseModel<ItemModel>::Field::updatedAt);
    const auto &itemID = std::cref(BaseModel<ItemModel>::Field::id);
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
        .order_by(std::make_pair(std::cref(orderByItemField), false), std::make_pair(std::cref(itemID), false))
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

    QuerySet qsItem(ItemModel::tableName, "_item", true, true);
    qsItem.filter(BaseModel<ItemModel>::Field::id.getFullFieldName(), stringId)
        .jsonFields(addExtraQuotes(ItemModel().fieldsJsonObject()));

    QuerySet qsMedia(MediaModel::tableName, 0, std::string("_media"));
    QuerySet qsTag(TagModel::tableName, 0, std::string("_tag"));
    qsMedia.join(ItemModel())
        .filter(BaseModel<ItemModel>::Field::id.getFullFieldName(), stringId)
        .order_by(std::make_pair(std::cref(MediaModel::Field::sort), true))
        .only(MediaModel::allSetFields())
        .functions(Function(fmt::format("format_src(media.src, '{}') as src", APP_CLOUD_NAME)));
    qsTag.join(ItemModel())
        .filter(TagModel::Field::itemId.getFullFieldName(), std::string(stringId))
        .order_by(std::make_pair(std::cref(BaseModel<TagModel>::Field::updatedAt), false))
        .only(TagModel::allSetFields());

    executeSqlQuery(callbackPtr, QuerySet::buildQuery(std::move(qsMedia), std::move(qsItem), std::move(qsTag)));
}