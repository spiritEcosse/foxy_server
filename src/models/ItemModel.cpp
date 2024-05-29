//
// Created by ihor on 13.01.2024.
//
#include "ItemModel.h"
#include "MediaModel.h"
#include "ShippingRateModel.h"
#include "src/orm/QuerySet.h"
#include "src/utils/db/String.h"
#include "src/utils/env.h"
#include <fmt/core.h>

using namespace api::v1;

template<>
std::map<std::string, std::pair<std::string, std::string>, std::less<>> BaseModel<ItemModel>::joinMap = {
    {MediaModel::tableName, {BaseModel<ItemModel>::Field::id.getFullFieldName(), MediaModel::Field::itemId.getFullFieldName()}},
    {ShippingRateModel::tableName,
     {ItemModel::Field::shippingProfileId.getFullFieldName(),
      ShippingRateModel::Field::shippingProfileId.getFullFieldName()}},
};

std::vector<BaseField> ItemModel::fields() {
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


std::vector<std::pair<BaseField,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
ItemModel::getObjectValues() const {
    std::vector<std::pair<BaseField,
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
    QuerySet qsCount(ItemModel::tableName, "total", false, true);
    return std::move(
        qsCount.filter(Field::enabled.getFullFieldName(), std::string("true"), false, std::string("="))
            .functions(Function(fmt::format("count(*)::integer"))));
}

std::string ItemModel::sqlSelectList(int page, int limit, [[maybe_unused]] const std::map<std::string, std::string, std::less<>> &params) {
    std::string app_cloud_name;
    getenv("APP_CLOUD_NAME", app_cloud_name);

    auto orderByItem = BaseModel<ItemModel>::Field::updatedAt;
    auto mediaSort = MediaModel::Field::sort.getFullFieldName();
    auto itemID = BaseModel<ItemModel>::Field::id;
    auto mediaItemID = MediaModel::Field::itemId.getFullFieldName();

    QuerySet qsCount = ItemModel().qsCount();
    QuerySet qsPage = ItemModel().qsPage(page, limit);

    QuerySet qs(ItemModel::tableName, limit, "data");
    qs.distinct(orderByItem, itemID)
        .join(MediaModel())
        .filter(ItemModel::Field::enabled.getFullFieldName(),
                std::string("true"),
                false,
                std::string("="),
                std::string("AND"))
        .filter(mediaSort,
                std::string(fmt::format("(SELECT MIN({}) FROM {} WHERE {} = {})",
                                        mediaSort,
                                        MediaModel::tableName,
                                        BaseModel<ItemModel>::Field::id.getFullFieldName(),
                                        mediaItemID)),
                false)
        .order_by(std::make_pair(orderByItem, false), std::make_pair(itemID, false))
        .only(allSetFields())
        .functions(Function(fmt::format("format_src(media.src, '{}') as src", app_cloud_name)))
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit));
    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}

std::string ItemModel::sqlSelectOne(const std::string &field,
                                    const std::string &value,
                                    [[maybe_unused]] const std::map<std::string, std::string, std::less<>> &params) {
    std::string app_cloud_name;
    getenv("APP_CLOUD_NAME", app_cloud_name);

    QuerySet qsItem(tableName, "_item", true, true);
    qsItem.filter(field, std::string(value)).jsonFields(addExtraQuotes(fieldsJsonObject()));

    QuerySet qsMedia(MediaModel::tableName, 0, std::string("_media"));
    std::string itemField = field;
    if(field == BaseModel::Field::id.getFullFieldName())
        itemField = MediaModel::Field::itemId.getFullFieldName();
    qsMedia.join(ItemModel())
        .filter(itemField, std::string(value))
        .order_by(std::make_pair(MediaModel::Field::sort, true))
        .only(MediaModel().allSetFields())
        .functions(Function(fmt::format("format_src(media.src, '{}') as src", app_cloud_name)));
    return QuerySet::buildQuery(std::move(qsMedia), std::move(qsItem));
}
