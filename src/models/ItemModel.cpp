//
// Created by ihor on 13.01.2024.
//
#include "ItemModel.h"
#include "MediaModel.h"
#include "ShippingRateModel.h"
#include "ShippingProfileModel.h"
#include "BasketItemModel.h"
#include "QuerySet.h"
#include "StringUtils.h"
#include "env.h"
#include <fmt/core.h>

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> ItemModel::joinMap() const {
    return {{MediaModel::tableName,
             {BaseModel<ItemModel>::Field::id.getFullFieldName(), MediaModel::Field::itemId.getFullFieldName()}},
            {ShippingRateModel::tableName,
             {ItemModel::Field::shippingProfileId.getFullFieldName(),
              ShippingRateModel::Field::shippingProfileId.getFullFieldName()}},
            {ShippingProfileModel::tableName,
             {ItemModel::Field::shippingProfileId.getFullFieldName(),
              BaseModel<ShippingProfileModel>::Field::id.getFullFieldName()}},
            {BasketItemModel::tableName,
             {BaseModel<ItemModel>::Field::id.getFullFieldName(), BasketItemModel::Field::itemId.getFullFieldName()}}};
}

std::vector<std::pair<BaseField,
                      std::variant<int,
                                   bool,
                                   std::string,
                                   std::vector<std::string>,
                                   std::chrono::system_clock::time_point,
                                   dec::decimal<2>>>>
ItemModel::getObjectValues() const {
    return {
        {Field::title, title},
        {Field::description, description},
        {Field::metaDescription, metaDescription},
        {Field::slug, slug},
        {Field::shippingProfileId, shippingProfileId},
        {Field::enabled, enabled},
        {Field::price, price},
    };
}

QuerySet ItemModel::qsCount() {
    QuerySet qsCount(ItemModel::tableName, "total", false, true);
    return std::move(qsCount.filter(Field::enabled.getFullFieldName(), std::string("true"), false, std::string("="))
                         .functions(Function("count(*)::integer")));
}

std::string ItemModel::sqlSelectList(int page,
                                     int limit,
                                     [[maybe_unused]] const std::map<std::string, std::string, std::less<>> &params) {
    std::string app_cloud_name;
    getenv("APP_CLOUD_NAME", app_cloud_name);
    std::string app_bucket_host;
    getenv("APP_BUCKET_HOST", app_bucket_host);

    auto orderByItem = BaseModel<ItemModel>::Field::updatedAt;
    std::string media_image = "media_image";
    std::string media_video = "media_video";
    auto mediaSort = fmt::format(R"("{}".{})", media_image, MediaModel::Field::sort.getFieldName());
    auto itemID = BaseModel<ItemModel>::Field::id;
    auto mediaItemID = MediaModel::Field::itemId.getFullFieldName();

    QuerySet qsCount = ItemModel().qsCount();
    QuerySet qsPage = ItemModel().qsPage(page, limit);

    QuerySet qs(ItemModel::tableName, limit, "data");
    qs.distinct(orderByItem, itemID)
        .join(MediaModel(), media_image, fmt::format(" AND {}.type = 'image'", media_image))
        .left_join(MediaModel(), media_video, fmt::format(" AND {}.type = 'video'", media_video))
        .filter(ItemModel::Field::enabled.getFullFieldName(),
                std::string("true"),
                false,
                std::string("="),
                std::string("AND"))
        .filter(mediaSort,
                std::string(fmt::format("(SELECT MIN({}) FROM {} WHERE {} = {})",
                                        MediaModel::Field::sort.getFullFieldName(),
                                        MediaModel::tableName,
                                        BaseModel<ItemModel>::Field::id.getFullFieldName(),
                                        mediaItemID)),
                false)
        .order_by(std::make_pair(orderByItem, false), std::make_pair(itemID, false))
        .only(allSetFields())
        .functions(Function(fmt::format("format_src({}.src, '{}') as src", media_image, app_cloud_name)))
        .functions(Function(fmt::format("format_src({}.src, '{}') as src_video", media_video, app_bucket_host)))
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
