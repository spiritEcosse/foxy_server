//
// Created by ihor on 14.01.2024.
//

#include "BaseModel.h"
#include "src/models/ItemModel.h"
#include "src/models/PageModel.h"
#include "src/models/UserModel.h"
#include "src/models/MediaModel.h"

using namespace api::v1;

std::string timePointToString(std::chrono::system_clock::time_point tp) {
    auto time_t = std::chrono::system_clock::to_time_t(tp);

    struct tm local_time {};

    localtime_r(&time_t, &local_time);

    std::stringstream ss;
    ss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");

    auto duration = tp.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

    ss << '.' << std::setfill('0') << std::setw(6) << milliseconds.count();

    return ss.str();
}

template<class T>
std::string BaseModel<T>::sqlDelete(int id) {
    return "DELETE FROM \"" + T::tableName + "\" WHERE " + T::primaryKey + " = " + std::to_string(id) + ";";
}

template<class T>
std::string BaseModel<T>::sqlInsert(const T &item) {
    std::string sql = "INSERT INTO \"" + T::tableName + "\" (" + T::fieldsToString() + ") VALUES (";
    for(const auto &[key, value]: item.getObjectValues()) {
        std::visit(
            [&sql](const auto &arg) {
                std::string data;
                using Type = std::decay_t<decltype(arg)>;
                if constexpr(std::is_same_v<Type, std::chrono::system_clock::time_point>) {
                    data = timePointToString(arg);
                } else if constexpr(std::is_same_v<Type, int>) {
                    data = std::to_string(arg);
                } else if constexpr(std::is_same_v<Type, bool>) {
                    data = arg ? "true" : "false";
                } else {
                    data = arg;
                }
                sql.append("'").append(data).append("',");
            },
            value);
    }
    sql.pop_back();
    sql.append(")");
    sql.append(" ON CONFLICT DO NOTHING RETURNING json_build_object(" + T::fieldsJsonObject() + ")");
    return sql;
}

template<class T>
std::string BaseModel<T>::sqlInsertMultiple(const std::vector<T> &items) {
    std::string sql = "INSERT INTO \"" + T::tableName + "\" (" + T::fieldsToString() + ") VALUES ";
    for(const auto &item: items) {
        sql.append("(");

        for(const auto &[key, value]: item.getObjectValues()) {
            std::visit(
                [&sql](const auto &arg) {
                    std::string data;
                    using Type = std::decay_t<decltype(arg)>;
                    if constexpr(std::is_same_v<Type, std::chrono::system_clock::time_point>) {
                        data = timePointToString(arg);
                    } else if constexpr(std::is_same_v<Type, int>) {
                        data = std::to_string(arg);
                    } else if constexpr(std::is_same_v<Type, bool>) {
                        data = arg ? "true" : "false";
                    } else {
                        data = arg;
                    }
                    sql.append("'").append(data).append("',");
                },
                value);
        }
        sql.pop_back();
        sql.append("),");
    }
    sql.pop_back();
    sql.append(" RETURNING json_build_object(" + T::fieldsJsonObject() + ")");
    return sql;
}

template<class T>
std::string BaseModel<T>::fieldsToString() {
    std::stringstream ss;
    auto fieldNames = T::fields();
    for(const auto &fieldName: fieldNames) {
        ss << fieldName;
        if(&fieldName != &fieldNames.back()) {
            ss << ", ";
        }
    }
    return ss.str();
}

template<class T>
std::string
BaseModel<T>::sqlSelectList(int page, int limit, const std::unordered_map<std::string, std::string> &params) {
    std::string pageStr = std::to_string(page);
    std::string limitStr = std::to_string(limit);

    std::string sqlItems;
    sqlItems += "SELECT * FROM \"" + T::tableName + "\" as t1 \n";
    if(!params.empty()) {
        sqlItems += "    WHERE ";
    }
    for(const auto &[key, value]: params) {
        sqlItems.append(key).append(" = '").append(value).append("' AND ");
    }

    // Remove the last " AND "
    if(sqlItems.substr(sqlItems.length() - 5) == " AND ") {
        sqlItems = sqlItems.substr(0, sqlItems.length() - 5);
    }
    sqlItems += "    ORDER BY t1." + T::orderBy + ", t1." + Field::id + " DESC \n";

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

template<class T>
std::string BaseModel<T>::fieldsJsonObject() {
    std::stringstream ss;
    auto fieldNames = T::fullFields();
    for(const auto &fieldName: fieldNames) {
        ss << "\'" << fieldName << "\', " << fieldName;
        if(&fieldName != &fieldNames.back()) {
            ss << ", ";
        }
    }
    return ss.str();
}

template<class T>
std::string BaseModel<T>::sqlSelectOne(const std::string &field, const std::string &value) {
    std::string sql = "SELECT (SELECT "
                      "json_build_object(" +
                      T::fieldsJsonObject() + ") FROM \"" + T::tableName + "\" where " + field + " = \'" + value +
                      "\') as " + T::tableName;
    return sql;
}

template<class T>
std::string BaseModel<T>::sqlUpdate(const T &item) {
    std::string sql = "UPDATE \"" + T::tableName + "\" SET ";
    for(const auto &[key, value]: item.getObjectValues()) {
        std::visit(
            [&sql, &key](const auto &arg) {  // check it in c++20
                std::string data;
                using Type = std::decay_t<decltype(arg)>;
                if constexpr(std::is_same_v<Type, std::chrono::system_clock::time_point>) {
                    data = timePointToString(arg);
                } else if constexpr(std::is_same_v<Type, int>) {
                    data = std::to_string(arg);
                } else if constexpr(std::is_same_v<Type, bool>) {
                    data = arg ? "true" : "false";
                } else {
                    data = arg;
                }
                sql.append(key).append(" = '").append(data).append("',");
            },
            value);
    }
    sql.pop_back();
    sql.append(" WHERE " + T::primaryKey + " = " + std::to_string(item.id) + " RETURNING json_build_object(" +
               T::fieldsJsonObject() + ")");
    return sql;
}

template class api::v1::BaseModel<PageModel>;
template class api::v1::BaseModel<ItemModel>;
template class api::v1::BaseModel<UserModel>;
template class api::v1::BaseModel<MediaModel>;
