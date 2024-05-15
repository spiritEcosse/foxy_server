//
// Created by ihor on 13.01.2024.
//
#include "ItemModel.h"
#include "MediaModel.h"
#include "ShippingProfileModel.h"
#include "ShippingRateModel.h"
#include "CountriesIpsModel.h"
#include "src/orm/QuerySet.h"
#include "src/utils/db/String.h"
#include "src/utils/env.h"
#include <fmt/core.h>

using namespace api::v1;

std::vector<std::string> ItemModel::fields() {
    return {
        Field::title,
        Field::description,
        Field::metaDescription,
        Field::slug,
        Field::shippingProfileId,
        Field::enabled,
        Field::price,
    };
}

std::vector<std::string> ItemModel::fullFields() {
    return {
        Field::id,
        Field::title,
        Field::enabled,
        Field::description,
        Field::metaDescription,
        Field::slug,
        Field::shippingProfileId,
        Field::createdAt,
        Field::updatedAt,
        Field::price,
    };
}

std::vector<std::pair<std::string,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
ItemModel::getObjectValues() const {
    std::vector<std::pair<std::string,
                          std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
        baseValues = {};
    baseValues.emplace_back(Field::title, title);
    baseValues.emplace_back(Field::description, description);
    baseValues.emplace_back(Field::metaDescription, metaDescription);
    baseValues.emplace_back(Field::slug, slug);
    baseValues.emplace_back(Field::shippingProfileId, shippingProfileId);
    baseValues.emplace_back(Field::enabled, enabled);
    baseValues.emplace_back(Field::price, price);
    return baseValues;
}

QuerySet ItemModel::qsCount() {
    QuerySet qsCount(ItemModel::tableName, "count", false, true);
    return std::move(qsCount
                         .filter(ItemModel::tableName + "." + ItemModel::Field::enabled,
                                 std::string("true"),
                                 false,
                                 std::string("="))
                         .only({fmt::format("count(*)::integer")}));
}

std::string ItemModel::sqlSelectList(int page, int limit) {
    std::string app_cloud_name;
    getenv("APP_CLOUD_NAME", app_cloud_name);

    auto orderByItemField = fmt::format("{}.{}", ItemModel::tableName, ItemModel::orderBy);
    auto mediaSort = fmt::format("{}.{}", MediaModel::tableName, MediaModel::Field::sort);
    auto itemID = fmt::format("{}.{}", ItemModel::tableName, ItemModel::Field::id);
    auto mediaItemID = fmt::format("{}.{}", MediaModel::tableName, MediaModel::Field::itemId);

    QuerySet qsCount = ItemModel::qsCount();
    QuerySet qsPage = ItemModel::qsPage(page, limit);

    QuerySet qs(ItemModel::tableName, limit, "data");
    qs.distinct(orderByItemField, itemID)
        .join(MediaModel::tableName,
              ItemModel::tableName + "." + ItemModel::Field::id + " = " + MediaModel::tableName + "." +
                  MediaModel::Field::itemId)
        .filter(ItemModel::tableName + "." + ItemModel::Field::enabled,
                std::string("true"),
                false,
                std::string("="),
                std::string("AND"))
        .filter(mediaSort,
                std::string(fmt::format("(SELECT MIN({}) FROM {} WHERE {} = {})",
                                        mediaSort,
                                        MediaModel::tableName,
                                        itemID,
                                        mediaItemID)),
                false)
        .order_by(std::make_pair(orderByItemField, false), std::make_pair(itemID, false))
        .only({ItemModel::fullFieldsWithTableToString(),
               fmt::format("format_src(media.src, '{}') as src", app_cloud_name)})
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit));
    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}

std::string ItemModel::sqlSelectOne(const std::string &field,
                                    const std::string &value,
                                    const std::map<std::string, std::string, std::less<>> &params) {
    std::string app_cloud_name;
    getenv("APP_CLOUD_NAME", app_cloud_name);

    QuerySet qsItem(tableName, "_item", true, true);
    qsItem.filter(fmt::format("{}.{}", ItemModel::tableName, field), std::string(value))
        .jsonFields(addExtraQuotes(ItemModel::fieldsJsonObject()));

    QuerySet qsMedia(MediaModel::tableName, 0, std::string("_media"));
    std::string itemField = ItemModel::tableName + "." + field;
    if(field == Field::id)
        itemField = MediaModel::tableName + "." + MediaModel::Field::itemId;
    qsMedia
        .join(ItemModel::tableName,
              ItemModel::tableName + "." + ItemModel::Field::id + " = " + MediaModel::tableName + "." +
                  MediaModel::Field::itemId)
        .filter(itemField, std::string(value))
        .order_by(std::make_pair(MediaModel::tableName + "." + MediaModel::Field::sort, true))
        .only({MediaModel::fullFieldsWithTableToString(),
               fmt::format("format_src(media.src, '{}') as src", app_cloud_name)});
    return QuerySet::buildQuery(std::move(qsMedia), std::move(qsItem));
}
