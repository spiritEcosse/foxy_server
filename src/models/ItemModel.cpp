//
// Created by ihor on 13.01.2024.
//
#include "ItemModel.h"
#include "MediaModel.h"

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

std::string ItemModel::sqlSelectList(int page,
                                     int limit,
                                     [[maybe_unused]] const std::unordered_map<std::string, std::string> &params) {
    std::string pageStr = std::to_string(page);
    std::string limitStr = std::to_string(limit);

    std::string sqlItems;
    sqlItems += "    SELECT DISTINCT ON (t1." + orderBy + ", t1.id) t1.*, t2." + MediaModel::Field::src;
    sqlItems += "    FROM " + tableName + " as t1 ";
    sqlItems += "    INNER JOIN " + MediaModel::tableName + " as t2 ON t1."  + Field::id + " = t2." + MediaModel::Field::itemId;
    sqlItems += "    WHERE t2." + MediaModel::Field::sort + " = 1 and t1." + Field::enabled + " = true";
    sqlItems += "    ORDER BY t1." + orderBy + ", t1." + Field::id + " DESC ";
    std::string sql = "WITH items AS (";
    sql += sqlItems;
    sql += "), ";
    sql += "item_count AS ( ";
    sql += "    SELECT count(*)::integer as count FROM items ";
    sql += "), ";
    sql += "valid_page AS ( ";
    sql += "    SELECT GetValidPage(" + pageStr + ", " + limitStr + ", (SELECT count FROM item_count)) as page ";
    sql += ") ";
    sql += "SELECT ";
    sql += "   (SELECT page FROM valid_page) as page, ";
    sql += "   (SELECT count FROM item_count) as count, ";
    sql += "   (SELECT json_agg(t.*) FROM ( ";
    sql += sqlItems;
    sql += "        OFFSET ((SELECT page FROM valid_page) - 1) * " + limitStr + " LIMIT " + limitStr;
    sql += "    ) as t ";
    sql += ") as items;";

    return sql;
}

std::string ItemModel::sqlSelectOne(const std::string &field, const std::string &value) {
    std::string sql = BaseModel::sqlSelectOne(field, value);
    sql += ", (SELECT json_agg(t2.*) FROM (SELECT " + MediaModel::fieldsToString() + "  FROM " + MediaModel::tableName + " WHERE " + field + " = " + value +
           " ORDER "
           "BY sort) as t2) as media";
    return sql;
}
