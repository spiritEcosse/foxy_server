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
#include "src/models/BasketModel.h"
#include "src/models/BasketItemModel.h"
#include "src/models/ShippingProfileModel.h"
#include "src/models/ShippingRateModel.h"
#include "src/models/OrderModel.h"
#include "src/models/CountriesIpsModel.h"
#include "src/models/CountryModel.h"
#include "src/orm/QuerySet.h"
#include "src/utils/db/String.h"
#include "decimal.h"

using namespace api::v1;

std::string timePointToString(std::chrono::system_clock::time_point tp)
{
    auto time_t = std::chrono::system_clock::to_time_t(tp);

    struct tm local_time{};

    localtime_r(&time_t, &local_time);

    std::string time_string = fmt::format("{:%Y-%m-%d %H:%M:%S}", local_time);

    auto duration = tp.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

    return fmt::format("{}.{}", time_string, milliseconds.count());
}

template<class T>
std::string BaseModel<T>::sqlDelete(int id)
{
    return "DELETE FROM \"" + T::tableName + "\" WHERE " + T::getPrimaryKeyFullName() + " = " + std::to_string(id) +
        ";";
}

template<class T>
std::string BaseModel<T>::sqlDeleteMultiple(const std::vector<int> &ids)
{
    std::string sql = "DELETE FROM \"" + T::tableName + "\" WHERE " + T::getPrimaryKeyFullName() + " IN (";
    for (const auto &_id: ids) {
        sql.append(std::to_string(_id)).append(",");
    }
    sql.pop_back();
    sql.append(");");
    return sql;
}

template<class T>
std::string BaseModel<T>::sqlInsertSingle(const T &item)
{
    std::string sql = " (";
    for (const auto &[key, value]: item.getObjectValues()) {
        std::visit(
            [&sql](const auto &arg)
            {
                std::string data;
                using Type = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<Type, std::chrono::system_clock::time_point>) {
                    data = timePointToString(arg);
                }
                else if constexpr (std::is_same_v<Type, std::string>) {
                    data = addExtraQuotes(arg);
                }
                else if constexpr (std::is_same_v<Type, int>) {
                    data = std::to_string(arg);
                }
                else if constexpr (std::is_same_v<Type, bool>) {
                    data = arg ? "true" : "false";
                }
                else if constexpr (std::is_same_v<Type, dec::decimal<2>>) {
                    std::stringstream ss;
                    ss << arg;
                    data = ss.str();
                }
                else {
                    data = arg;
                }
                if (data != "Null") {
                    data = addExtraQuotes(data);
                    sql.append("'").append(data).append("',");
                }
                else {
                    sql.append(data).append(",");
                }
            },
            value);
    }
    sql.pop_back();
    sql.append(")");
    return sql;
}

template<class T>
std::string BaseModel<T>::sqlInsert(const T &item)
{
    return fmt::format(
        R"(INSERT INTO "{}" ({}) VALUES {} RETURNING json_build_object({}))",
        T::tableName, T::fieldsToString(), sqlInsertSingle(item), T::fieldsJsonObject());
}

template<class T>
std::string BaseModel<T>::sqlInsertMultiple(const std::vector<T> &items)
{
    std::string sql = "INSERT INTO \"" + T::tableName + "\" (" + T::fieldsToString() + ") VALUES ";
    for (const auto &item: items) {
        sql.append(sqlInsertSingle(item)).append(",");
    }
    sql.pop_back();
    sql.append(" RETURNING json_build_object(" + T::fieldsJsonObject() + ")");
    return sql;
}

template<class T>
void BaseModel<T>::sqlUpdateSingle(const T &item, ModelFieldKeyHash &uniqueColumns)
{
    std::string sql;

    for (const auto &[key, value]: item.getObjectValues()) {
        std::visit(
            [&item, &uniqueColumns, key](const auto &arg)
            {
                std::string data;
                using Type = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<Type, std::chrono::system_clock::time_point>) {
                    data = timePointToString(arg);
                }
                else if constexpr (std::is_same_v<Type, std::string>) {
                    data = addExtraQuotes(arg);
                }
                else if constexpr (std::is_same_v<Type, int>) {
                    data = std::to_string(arg);
                }
                else if constexpr (std::is_same_v<Type, bool>) {
                    data = arg ? "true" : "false";
                }
                else if constexpr (std::is_same_v<Type, dec::decimal<2>>) {
                    std::stringstream ss;
                    ss << arg;
                    data = ss.str();
                }
                else {
                    data = arg;
                }
                if (data != "Null") {
                    data = addExtraQuotes(data);
                    uniqueColumns[key.getFieldName()].append(
                        fmt::format(R"( WHEN {} = {} THEN '{}' )", T::getPrimaryKeyFullName(), item.id, data));
                }
                else {
                    uniqueColumns[key.getFieldName()].append(
                        fmt::format(R"( WHEN {} = {} THEN {} )", T::getPrimaryKeyFullName(), item.id, data));
                }
            },
            value);
    }
}

template<class T>
std::string BaseModel<T>::sqlUpdate(T &&item)
{
    std::vector<T> items;
    items.push_back(std::move(item));
    return sqlUpdateMultiple(items);
}

template<class T>
std::string BaseModel<T>::sqlUpdateMultiple(const std::vector<T> &items)
{
    std::string sql = fmt::format(R"(UPDATE "{}" SET )", T::tableName);
    std::string ids;
    ModelFieldKeyHash uniqueColumns;

    for (const auto &item: items) {
        sqlUpdateSingle(item, uniqueColumns);
        ids.append(fmt::format("{},", item.id));
    }
    ids.pop_back();
    for (const auto &[key, value]: uniqueColumns) {
        sql.append(fmt::format("{} = CASE {} ELSE {} END,", key, value, key));
    }
    sql.pop_back();

    sql.append(fmt::format(" WHERE {} IN ({}) ", T::getPrimaryKeyFullName(), ids));
    sql.append(fmt::format(" RETURNING json_build_object({});", T::fieldsJsonObject()));
    return sql;
}

template<class T>
std::string BaseModel<T>::fieldsToString()
{
    std::stringstream ss;
    for (auto fieldNames = T::fields(); const auto &fieldName: fieldNames) {
        ss << fieldName.getFieldName();
        if (&fieldName != &fieldNames.back()) {
            ss << ", ";
        }
    }
    return ss.str();
}

template<class T>
std::string BaseModel<T>::fullFieldsWithTableToString()
{
    std::stringstream ss;
    for (auto fieldNames = T::fullFields(); const auto &fieldName: fieldNames) {
        ss << fieldName.getFullFieldName();
        if (&fieldName != &fieldNames.back()) {
            ss << ", ";
        }
    }
    return ss.str();
}

template<class T>
std::string BaseModel<T>::sqlSelectList(int page, int limit)
{
    QuerySet qsCount = std::move(T::qsCount());
    QuerySet qsPage = std::move(T::qsPage(page, limit));

    QuerySet qs(T::tableName, limit, "data");
    qs.offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
        .order_by(std::make_pair(T::getOrderByFullName(), false), std::make_pair(T::getPrimaryKeyFullName(), false));
    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}

template<class T>
QuerySet BaseModel<T>::qsCount()
{
    QuerySet qsCount(T::tableName, "total", false, true);
    return std::move(qsCount.only({fmt::format("count(*)::integer")}));
}

template<class T>
QuerySet BaseModel<T>::qsPage(int page, int limit)
{
    QuerySet qsCount = std::move(T::qsCount());
    QuerySet qsPage(ItemModel::tableName, "_page", false, true);
    return std::move(
        qsPage.only({fmt::format("GetValidPage({}, {}, (SELECT * FROM {}))", page, limit, qsCount.alias())}));
}

template<class T>
std::string BaseModel<T>::fieldsJsonObject()
{
    std::stringstream ss;
    for (auto fieldNames = T::fullFields(); const auto &fieldName: fieldNames) {
        ss << "\'" << fieldName.getFieldName() << "\', " << fieldName.getFullFieldName();
        if (&fieldName != &fieldNames.back()) {
            ss << ", ";
        }
    }
    return ss.str();
}

template<class T>
std::string BaseModel<T>::sqlSelectOne(const std::string &field,
                                       const std::string &value,
                                       [[maybe_unused]] const std::map<std::string, std::string, std::less<>> &params)
{
    QuerySet qs(T::tableName, T::tableName, true);
    qs.jsonFields(addExtraQuotes(T::fieldsJsonObject())).filter(field, std::string(value));
    return qs.buildSelect();
}

template
class api::v1::BaseModel<PageModel>;
template
class api::v1::BaseModel<ItemModel>;
template
class api::v1::BaseModel<UserModel>;
template
class api::v1::BaseModel<MediaModel>;
template
class api::v1::BaseModel<ShippingProfileModel>;
template
class api::v1::BaseModel<ShippingRateModel>;
template
class api::v1::BaseModel<CountryModel>;
template
class api::v1::BaseModel<CountriesIpsModel>;
template
class api::v1::BaseModel<OrderModel>;
template
class api::v1::BaseModel<BasketItemModel>;
template
class api::v1::BaseModel<BasketModel>;
