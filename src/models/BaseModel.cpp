//
// Created by ihor on 14.01.2024.
//

#include <ctime>
#include <sstream>
#include <fmt/core.h>
#include <fmt/chrono.h>
#include "BaseModel.h"
#include "src/models/ItemModel.h"
#include "src/models/PageModel.h"
#include "src/models/UserModel.h"
#include "src/models/MediaModel.h"
#include "src/orm/QuerySet.h"
#include "src/utils/db/String.h"

using namespace api::v1;

std::string timePointToString(std::chrono::system_clock::time_point tp) {
    auto time_t = std::chrono::system_clock::to_time_t(tp);

    struct tm local_time {};
    localtime_r(&time_t, &local_time);

    std::string time_string = fmt::format("{:%Y-%m-%d %H:%M:%S}", local_time);

    auto duration = tp.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

    return fmt::format("{}.{}", time_string, milliseconds.count());
}

template<class T>
std::string BaseModel<T>::sqlDelete(int id) {
    return "DELETE FROM \"" + T::tableName + "\" WHERE " + T::primaryKey + " = " + std::to_string(id) + ";";
}

template<class T>
std::string BaseModel<T>::sqlInsertSingle(const T &item) {
    std::string sql = " (";
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
    return sql;
}

template<class T>
std::string BaseModel<T>::sqlInsert(const T &item) {
    std::string sql = "INSERT INTO \"" + T::tableName + "\" (" + T::fieldsToString() + ") VALUES ";
    sql += sqlInsertSingle(item);
    sql.append(" RETURNING json_build_object(" + T::fieldsJsonObject() + ")");
    return sql;
}

template<class T>
std::string BaseModel<T>::sqlInsertMultiple(const std::vector<T> &items) {
    std::string sql = "INSERT INTO \"" + T::tableName + "\" (" + T::fieldsToString() + ") VALUES ";
    for(const auto &item: items) {
        sql.append(sqlInsertSingle(item)).append(",");
    }
    sql.pop_back();
    sql.append(" RETURNING json_build_object(" + T::fieldsJsonObject() + ")");
    return sql;
}

template<class T>
void BaseModel<T>::sqlUpdateSingle(const T &item, ModelFieldKeyHash &uniqueColumns) {
    std::string sql;
    // Loop over each field in the item

    for(const auto &[key, value]: item.getObjectValues()) {
        std::visit(
            [&item, &uniqueColumns, key](const auto &arg) {
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
                uniqueColumns[key].append(fmt::format("WHEN {} = {} THEN '{}' ", T::primaryKey, item.id, data));
            },
            value);
    }
}

template<class T>
std::string BaseModel<T>::sqlUpdate(T &&item) {
    std::vector<T> items;
    items.push_back(std::move(item));
    return sqlUpdateMultiple(items);
}

template<class T>
std::string BaseModel<T>::sqlUpdateMultiple(const std::vector<T> &items) {
    std::string sql = fmt::format("UPDATE \"{}\" SET ", T::tableName);
    std::string ids;
    ModelFieldKeyHash uniqueColumns;

    for(const auto &item: items) {
        sqlUpdateSingle(item, uniqueColumns);
        ids.append(fmt::format("{},", item.id));
    }
    ids.pop_back();
    for(const auto &[key, value]: uniqueColumns) {
        sql.append(fmt::format("{} = CASE {} ELSE {} END,", key, value, key));
    }
    sql.pop_back();

    sql.append(fmt::format(" WHERE {} IN ({}) ", T::primaryKey, ids));
    sql.append(fmt::format(" RETURNING json_build_object({});", T::fieldsJsonObject()));
    std::cout << sql << std::endl;
    return sql;
}

template<class T>
std::string BaseModel<T>::fieldsToString() {
    std::stringstream ss;
    for(auto fieldNames = T::fields(); const auto &fieldName: fieldNames) {
        ss << fieldName;
        if(&fieldName != &fieldNames.back()) {
            ss << ", ";
        }
    }
    return ss.str();
}

template<class T>
std::string BaseModel<T>::fullFieldsWithTableToString() {
    std::stringstream ss;
    for(auto fieldNames = T::fullFields(); const auto &fieldName: fieldNames) {
        ss << T::tableName << "." << fieldName;
        if(&fieldName != &fieldNames.back()) {
            ss << ", ";
        }
    }
    return ss.str();
}

template<class T>
std::string
BaseModel<T>::sqlSelectList(int page, int limit) {
    QuerySet qs(T::tableName, false, limit, page, true);
    qs.order_by(std::make_pair(T::tableName + "." + T::orderBy, false), std::make_pair(T::tableName + "." + T::Field::id, false));
    return qs.buildSelect();
}

template<class T>
std::string BaseModel<T>::fieldsJsonObject() {
    std::stringstream ss;
    for(auto fieldNames = T::fullFields(); const auto &fieldName: fieldNames) {
        ss << "\'" << fieldName << "\', " << fieldName;
        if(&fieldName != &fieldNames.back()) {
            ss << ", ";
        }
    }
    return ss.str();
}

template<class T>
std::string BaseModel<T>::sqlSelectOne(const std::string &field, const std::string &value) {
    QuerySet qs(T::tableName, true);
    qs.jsonFields(addExtraQuotes(T::fieldsJsonObject()))
    .filter(std::make_pair(field, value));
    return qs.buildSelect();
}

template<class T>
void BaseModel<T>::checkMissingFields(const Json::Value& missingFields) const {
    if(!missingFields.empty()) {
        throw RequiredFieldsException(missingFields);
    }
}

template<class T>
void BaseModel<T>::validateField(const std::string& fieldName, const std::string_view& value, Json::Value& missingFields) const {
    if(value.empty()) {
        missingFields[fieldName] = fieldName + " is required";
    }
}

template class api::v1::BaseModel<PageModel>;
template class api::v1::BaseModel<ItemModel>;
template class api::v1::BaseModel<UserModel>;
template class api::v1::BaseModel<MediaModel>;
