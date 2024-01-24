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
    };
}

std::vector<std::string> ItemModel::fullFields() {
    return {Field::id,
            Field::title,
            Field::description,
            Field::metaDescription,
            Field::slug,
            Field::createdAt,
            Field::updatedAt};
}

std::vector<std::pair<std::string, std::variant<int, bool, std::string>>> ItemModel::getObjectValues() const {
    return {
        {Field::title, title},
        {Field::description, description},
        {Field::metaDescription, metaDescription},
        {Field::slug, slug},
    };
}

std::string ItemModel::sqlSelectList(int page,
                                     int limit,
                                     [[maybe_unused]] const std::unordered_map<std::string, std::string> &params) {
    std::string pageStr = std::to_string(page);
    std::string limitStr = std::to_string(limit);

    std::string sql = "SELECT ";
    sql += "(SELECT GetValidPage(" + pageStr + ", " + limitStr + ")) as page,\n";
    sql += "(SELECT count(*) FROM " + tableName + ") as count,\n";
    sql += "(SELECT json_agg(t.*) FROM (\n";
    sql += "    SELECT DISTINCT t1.*, t2.src FROM " + tableName +
           " as t1 INNER JOIN " + MediaModel::tableName + " as t2 ON t1." + Field::id + " = t2." + MediaModel::Field::itemId + " \n";
    sql += "    WHERE t2.sort = 1\n";
    sql += "    ORDER BY t1." + orderBy + " DESC \n";
    sql += "    OFFSET (GetValidPage(" + pageStr + ", " + limitStr + ") - 1) * " + limitStr + "\n";
    sql += "    LIMIT " + limitStr + ") as t) AS items;";

    return sql;
}

std::string ItemModel::sqlSelectOne(const std::string &field, const std::string &value) {
    std::string sql = BaseModel::sqlSelectOne(field, value);
    sql += ", (SELECT json_agg(t2.*) FROM (SELECT " + MediaModel::fieldsToString() + "  FROM " + MediaModel::tableName + " WHERE " + field + " = " + value +
           " ORDER "
           "BY sort) as t2) as media";
    return sql;
}
