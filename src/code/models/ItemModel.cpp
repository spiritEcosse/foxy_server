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

BaseModelImpl::JoinMap ItemModel::joinMap() const {
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

QuerySet ItemModel::qsCount() {
    QuerySet qsCount(tableName, "total", false, true);
    return std::move(qsCount.filter(&Field::enabled, true).functions(Function("count(*)::integer")));
}

std::string ItemModel::sqlSelectList(const int page,
                                     int limit,
                                     [[maybe_unused]] const std::map<std::string, std::string, std::less<>> &params) {
    const auto &orderByItem = &BaseModel::Field::updatedAt;
    std::string media_image = "media_image";
    std::string media_video = "media_video";
    const auto mediaSort = fmt::format(R"("{}".{})", media_image, MediaModel::Field::sort.getFieldName());
    const auto &itemID = &BaseModel::Field::id;
    auto mediaItemID = MediaModel::Field::itemId.getFullFieldName();

    QuerySet qsCount = ItemModel::qsCount();
    QuerySet qsPage = ItemModel::qsPage(page, limit);

    QuerySet qs(tableName, limit, "data");
    qs.distinct(orderByItem, itemID)
        .join(MediaModel(), media_image, fmt::format(" AND {}.type = 'image'", media_image))
        .left_join(MediaModel(), media_video, fmt::format(" AND {}.type = 'video'", media_video))
        .filter(&Field::enabled, true)
        // .filter(mediaSort,
        //         std::string(fmt::format("(SELECT MIN({}) FROM {} WHERE {} = {})",
        //                                 MediaModel::Field::sort.getFullFieldName(),
        //                                 MediaModel::tableName,
        //                                 BaseModel::Field::id.getFullFieldName(),
        //                                 mediaItemID)))
        .order_by(orderByItem, false)
        .order_by(itemID, false)
        .only(allSetFields())
        .functions(Function(fmt::format("format_src({}.src, '{}') as src", media_image, APP_CLOUD_NAME)))
        .functions(Function(fmt::format("format_src({}.src, '{}') as src_video", media_video, APP_BUCKET_HOST)))
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit));
    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}

std::string ItemModel::sqlSelectOne(const BaseField *field,
                                    const std::string &value,
                                    [[maybe_unused]] const std::map<std::string, std::string, std::less<>> &params) {
    QuerySet qsMedia(MediaModel::tableName, 0, MediaModel::tableName, false);
    qsMedia.filter(&MediaModel::Field::itemId, &BaseModel::Field::id)
        .functions(Function(
            fmt::format("json_agg(json_build_object({}) ORDER BY media.sort ASC)", MediaModel().fieldsJsonObject())));

    QuerySet qsItem(tableName, tableName, true, true);
    qsItem.filter(field, value)
        .jsonFields(addExtraQuotes(fieldsJsonObject()))
        .functions(Function(fmt::format(R"( 'media', COALESCE(({}), '[]'::json))", qsMedia.buildSelect())));

    return qsItem.buildSelectOne();
}
