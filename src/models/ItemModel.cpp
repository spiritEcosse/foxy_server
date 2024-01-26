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
    QuerySet qs(ItemModel::tableName);
    qs.distinct({ItemModel::tableName + "." + ItemModel::orderBy, ItemModel::tableName + "." + ItemModel::Field::id})
        .join(MediaModel::tableName, ItemModel::tableName + "." + ItemModel::Field::id + " = " + MediaModel::tableName + "." + MediaModel::Field::itemId)
        .order_by({{ItemModel::tableName + "." + ItemModel::orderBy, false}, {ItemModel::tableName + "." + ItemModel::Field::id, false}})
        .filter({{ItemModel::tableName + "." + ItemModel::Field::enabled, "true"}})
        .limit(limit)
        .only({ItemModel::fullFieldsWithTableToString(), MediaModel::tableName + "." + MediaModel::Field::src})
        .page(page);
    return qs.buildSelect();
}

std::string ItemModel::sqlSelectOne(const std::string &field, const std::string &value) {
    std::string sql = BaseModel::sqlSelectOne(field, value);
    sql += ", (SELECT json_agg(t2.*) FROM (SELECT " + MediaModel::fieldsToString() + "  FROM " + MediaModel::tableName + " WHERE " + field + " = " + value +
           " ORDER "
           "BY sort) as t2) as media";
    return sql;
}
