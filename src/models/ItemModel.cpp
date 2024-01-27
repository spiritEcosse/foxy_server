//
// Created by ihor on 13.01.2024.
//
#include "ItemModel.h"
#include "MediaModel.h"
#include "src/orm/QuerySet.h"

using namespace api::v1;

std::vector<std::string> ItemModel::fields() {
    return {
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
            Field::updatedAt
    };
}

std::vector<std::pair<std::string, std::variant<int, bool, std::string>>> ItemModel::getObjectValues() const {
    return {
        {Field::title, title},
        {Field::description, description},
        {Field::metaDescription, metaDescription},
        {Field::slug, slug},
        {Field::enabled, enabled},
    };
}

std::string ItemModel::sqlSelectList(int page, int limit) {
    QuerySet qs(ItemModel::tableName, false, limit, page, true);
    qs.distinct(ItemModel::tableName + "." + ItemModel::orderBy, ItemModel::tableName + "." + ItemModel::Field::id)
        .join(MediaModel::tableName, ItemModel::tableName + "." + ItemModel::Field::id + " = " + MediaModel::tableName + "." + MediaModel::Field::itemId)
        .order_by(std::make_pair(ItemModel::tableName + "." + ItemModel::orderBy, false), std::make_pair(ItemModel::tableName + "." + ItemModel::Field::id, false))
        .filter(std::make_pair(ItemModel::tableName + "." + ItemModel::Field::enabled, "true"))
        .only({ItemModel::fullFieldsWithTableToString(), MediaModel::tableName + "." + MediaModel::Field::src});
    return qs.buildSelect();
}

std::string ItemModel::sqlSelectOne(const std::string &field, const std::string &value) {
    std::string sql = BaseModel::sqlSelectOne(field, value);
    QuerySet qsItem(tableName, true);
    qsItem.jsonFields(ItemModel::fieldsJsonObject())
        .filter(std::make_pair(field, value));
    QuerySet qsMedia(MediaModel::tableName, false, 0, 0, false);
    qsMedia.filter(std::make_pair(MediaModel::Field::itemId, value))
        .order_by(std::make_pair(MediaModel::tableName + "." + MediaModel::Field::sort, true))
        .only({MediaModel::fullFieldsWithTableToString()});
    return qsMedia.addQuery(qsItem);
}
