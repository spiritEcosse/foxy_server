//
// Created by ihor on 14.01.2024.
//

#include <ctime>
#include <sstream>
#include <iomanip>
#include "BaseModel.h"
#include "src/models/ItemModel.h"
#include "src/models/PageModel.h"
#include "src/models/UserModel.h"
#include "src/models/MediaModel.h"
#include "src/orm/QuerySet.h"

using namespace api::v1;

std::string timePointToString(std::chrono::system_clock::time_point tp) {
    auto time_t = std::chrono::system_clock::to_time_t(tp);

    struct tm local_time {};

    localtime_r(&time_t, &local_time);

    std::ostringstream oss;
    oss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");

    auto duration = tp.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

    std::string time_string = oss.str();
    return time_string + "." + std::to_string(milliseconds.count());
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
    sql.append(" RETURNING json_build_object(" + T::fieldsJsonObject() + ")");
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
std::string BaseModel<T>::fullFieldsWithTableToString() {
    std::stringstream ss;
    auto fieldNames = T::fullFields();
    for(const auto &fieldName: fieldNames) {
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

std::string addExtraQuotes(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (c == '\'') {
            result += "''";
        } else {
            result += c;
        }
    }
    return result;
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
    QuerySet qs(T::tableName, true);
    qs.jsonFields(T::fieldsJsonObject())
    .filter(std::make_pair(field, value));
    return qs.buildSelect();
}

template<class T>
std::string BaseModel<T>::sqlUpdate(const T &item) {
    std::string sql = "SELECT update_item('UPDATE \"" + T::tableName + "\" SET ";
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
                sql.append(key).append(" = ''").append(data).append("'',");
            },
            value);
    }
    sql.pop_back();
    sql.append(" WHERE " + T::primaryKey + " = " + std::to_string(item.id) + " RETURNING json_build_object(" +
        addExtraQuotes(T::fieldsJsonObject()) + ")')");
    return sql;
}

template class api::v1::BaseModel<PageModel>;
template class api::v1::BaseModel<ItemModel>;
template class api::v1::BaseModel<UserModel>;
template class api::v1::BaseModel<MediaModel>;
