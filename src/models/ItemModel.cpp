//
// Created by ihor on 13.01.2024.
//
#include "ItemModel.h"
#include "MediaModel.h"
#include "src/orm/QuerySet.h"
#include "src/utils/db/String.h"
#include "src/utils/env.h"
#include <fmt/core.h>

using namespace api::v1;

std::vector<std::string> ItemModel::fields() {
    return {
        Field::updatedAt,
        Field::title,
        Field::description,
        Field::metaDescription,
        Field::slug,
        Field::enabled,
    };
}

std::vector<std::string> ItemModel::fullFields() {
    return {Field::id,
            Field::title,
            Field::enabled,
            Field::description,
            Field::metaDescription,
            Field::slug,
            Field::createdAt,
            Field::updatedAt};
}

std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
ItemModel::getObjectValues() const {
    auto baseValues = BaseModel::getObjectValues();
    baseValues.emplace_back(Field::title, title);
    baseValues.emplace_back(Field::description, description);
    baseValues.emplace_back(Field::metaDescription, metaDescription);
    baseValues.emplace_back(Field::slug, slug);
    baseValues.emplace_back(Field::enabled, enabled);
    return baseValues;
}

std::string ItemModel::sqlSelectList(int page, int limit) {
    std::string app_cloud_name;
    getenv("APP_CLOUD_NAME", app_cloud_name);

    auto orderByItemField = fmt::format("{}.{}", ItemModel::tableName, ItemModel::orderBy);
    auto mediaSort = fmt::format("{}.{}", MediaModel::tableName, MediaModel::Field::sort);
    auto itemID = fmt::format("{}.{}", ItemModel::tableName, ItemModel::Field::id);
    auto mediaItemID = fmt::format("{}.{}", MediaModel::tableName, MediaModel::Field::itemId);

    QuerySet qs(ItemModel::tableName, false, limit, page, true);
    qs.distinct(orderByItemField, itemID)
        .join(MediaModel::tableName,
              ItemModel::tableName + "." + ItemModel::Field::id + " = " + MediaModel::tableName + "." +
                  MediaModel::Field::itemId)
        .order_by(std::make_pair(ItemModel::tableName + "." + ItemModel::orderBy, false),
                  std::make_pair(ItemModel::tableName + "." + ItemModel::Field::id, false))
        .filter(ItemModel::tableName + "." + ItemModel::Field::enabled, std::string("true"))
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
    return qs.buildSelect();
}

std::string ItemModel::sqlSelectOne(const std::string &field, const std::string &value) {
    std::string app_cloud_name;
    getenv("APP_CLOUD_NAME", app_cloud_name);

    QuerySet qsItem(tableName, true);
    qsItem.jsonFields(addExtraQuotes(ItemModel::fieldsJsonObject())).filter(field, std::string(value));
    QuerySet qsMedia(MediaModel::tableName, false, 0, 0, false);
    std::string itemField = field;
    if(field == Field::id) {
        itemField = MediaModel::tableName + "." + MediaModel::Field::itemId;
    } else {
        itemField = ItemModel::tableName + "." + field;
    }
    qsMedia
        .join(ItemModel::tableName,
              ItemModel::tableName + "." + ItemModel::Field::id + " = " + MediaModel::tableName + "." +
                  MediaModel::Field::itemId)
        .filter(itemField, std::string(value))
        .order_by(std::make_pair(MediaModel::tableName + "." + MediaModel::Field::sort, true))
        .only({MediaModel::fullFieldsWithTableToString(),
               fmt::format("format_src(media.src, '{}') as src", app_cloud_name)});
    return qsMedia.addQuery(qsItem);
}
