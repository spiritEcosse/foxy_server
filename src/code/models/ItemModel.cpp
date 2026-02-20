#include "ItemModel.h"
#include "MediaModel.h"
#include "ShippingRateModel.h"
#include "ShippingProfileModel.h"
#include "BasketItemModel.h"
#include "QuerySet.h"
#include "SocialMediaModel.h"
#include "StringUtils.h"
#include "config.h"
#include <fmt/core.h>

using namespace api::v1;

BaseModelImpl::JoinMap ItemModel::joinMap() {
    return {{MediaModel::tableName, {&BaseModel::Field::id, &MediaModel::Field::itemId}},
            {ShippingRateModel::tableName, {&Field::shippingProfileId, &ShippingRateModel::Field::shippingProfileId}},
            {ShippingProfileModel::tableName, {&Field::shippingProfileId, &BaseModel<ShippingProfileModel>::Field::id}},
            {BasketItemModel::tableName, {&BaseModel::Field::id, &BasketItemModel::Field::itemId}}};
}

BaseModel<ItemModel>::SetMapFieldTypes ItemModel::getObjectValues() const {
    return {{&Field::title, title},
            {&Field::description, description},
            {&Field::metaDescription, metaDescription},
            {&Field::slug, slug},
            {&Field::shippingProfileId, shippingProfileId},
            {&Field::enabled, enabled},
            {&Field::price, price}};
}

QuerySet<ItemModel> ItemModel::qsCount() {
    QuerySet<ItemModel> qsCount("total", false, true);
    return std::move(qsCount.filter(&Field::enabled, true).functions(Function("count(*)::integer")));
}

std::string ItemModel::sqlSelectList(const int page,
                                     int limit,
                                     [[maybe_unused]] const std::map<std::string, std::string, std::less<>> &params) {
    const auto &orderByItem = &BaseModel::Field::updatedAt;
    auto qsPage = ItemModel::qsPage(page, limit);

    std::string media_video = "media_video";
    const auto &itemID = &BaseModel::Field::id;
    auto mediaItemID = MediaModel::Field::itemId.getFullFieldName();
    QuerySet<MediaModel> qsMedia = MediaModel::qsMediaMinSort();
    std::string mediaAlias = qsMedia.getAlias();

    QuerySet<ItemModel> qs(limit, "data");
    qs.join<MediaModel>(std::move(qsMedia),
                        "image_media",
                        fmt::format(R"(AND {0}.type = 'image' AND {0}.row_num = 1)", "image_media"))
        .left_join<MediaModel>(std::move(mediaAlias),
                               "video_media",
                               fmt::format(R"(AND {0}.type = 'video' AND {0}.row_num = 1)", "video_media"))
        .filter(&Field::enabled, true)
        .order_by(orderByItem, false)
        .order_by(itemID, false)
        .only(allSetFields())
        .functions(Function(fmt::format("format_src({}.src, '{}') as src", "image_media", getEnv("APP_CLOUD_NAME"))))
        .functions(Function(fmt::format("format_src({}.src, '{}') as src_video", "video_media", getEnv("APP_BUCKET_HOST"))))
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit));
    return BuildComplexQueries::buildQuery(qsCount(), std::move(qsPage), std::move(qs));
}

std::string ItemModel::sqlSelectOne(const BaseField *field,
                                    std::string &&value,
                                    [[maybe_unused]] const std::map<std::string, std::string, std::less<>> &params) {
    QuerySet<MediaModel> qsMedia(0, MediaModel::tableName, false);
    qsMedia.filter(&MediaModel::Field::itemId, &BaseModel::Field::id)
        .functions(Function(
            fmt::format("json_agg(json_build_object({}) ORDER BY media.sort ASC)", MediaModel().fieldsJsonObject())));

    QuerySet<ItemModel> qsItem(tableName, true, true);
    qsItem.filter(field, std::move(value))
        .jsonFields(addExtraQuotes(fieldsJsonObject()))
        .functions(
            Function(addExtraQuotes(fmt::format(R"( 'media', COALESCE(({}), '[]'::json))", qsMedia.buildSelect()))));

    return qsItem.buildSelectOne();
}
