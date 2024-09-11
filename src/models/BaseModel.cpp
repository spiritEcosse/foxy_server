//
// Created by ihor on 14.01.2024.
//

#include <ctime>
#include <sstream>
#include <fmt/core.h>
#include <fmt/chrono.h>
#include "BaseModel.h"
#include "ReviewModel.h"
#include "ItemModel.h"
#include "PageModel.h"
#include "UserModel.h"
#include "MediaModel.h"
#include "AddressModel.h"
#include "BasketModel.h"
#include "BasketItemModel.h"
#include "ShippingProfileModel.h"
#include "ShippingRateModel.h"
#include "OrderModel.h"
#include "CountriesIpsModel.h"
#include "CountryModel.h"
#include "QuerySet.h"
#include "StringUtils.h"
#include "decimal.h"
#include "FinancialDetailsModel.h"
#include "SocialMediaModel.h"

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
std::string BaseModel<T>::sqlDelete(int idKey) {
    return fmt::format(R"(DELETE FROM "{}" WHERE {} = {})", T::tableName, T::Field::id.getFullFieldName(), idKey);
}

template<class T>
std::string BaseModel<T>::sqlDeleteMultiple(const std::vector<int> &ids) {
    std::string sql = "DELETE FROM \"" + T::tableName + "\" WHERE " + T::Field::id.getFullFieldName() + " IN (";
    for(const auto &_id: ids) {
        sql.append(std::to_string(_id)).append(",");
    }
    sql.pop_back();
    sql.append(");");
    return sql;
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
                } else if constexpr(std::is_same_v<Type, std::string>) {
                    data = addExtraQuotes(arg);
                } else if constexpr(std::is_same_v<Type, int>) {
                    data = std::to_string(arg);
                } else if constexpr(std::is_same_v<Type, bool>) {
                    data = arg ? "true" : "false";
                } else if constexpr(std::is_same_v<Type, dec::decimal<2>>) {
                    std::stringstream ss;
                    ss << arg;
                    data = ss.str();
                } else if constexpr(std::is_same_v<Type, std::vector<std::string>>) {
                    // Handle vector of strings (e.g., for tags)
                    data = "{";  // Start array representation in SQL
                    for(const auto &str: arg) {
                        data.append(addExtraQuotes(str)).append(",");
                    }
                    if(!arg.empty()) {
                        data.pop_back();  // Remove trailing comma
                    }
                    data.append("}");  // End array representation in SQL
                } else {
                    data = arg;
                }
                if(data != "Null") {
                    sql.append("'").append(data).append("',");
                } else {
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
std::string BaseModel<T>::sqlInsert(const T &item) {
    return fmt::format(R"(INSERT INTO "{}" ({}) VALUES {} RETURNING json_build_object({}))",
                       T::tableName,
                       fieldsToString(),
                       sqlInsertSingle(item),
                       fieldsJsonObject());
}

template<class T>
std::string BaseModel<T>::sqlInsertMultiple(const std::vector<T> &items) {
    std::string sql = "INSERT INTO \"" + T::tableName + "\" (" + fieldsToString() + ") VALUES ";
    for(const auto &item: items) {
        sql.append(sqlInsertSingle(item)).append(",");
    }
    sql.pop_back();
    sql.append(" RETURNING json_build_object(" + fieldsJsonObject() + ")");
    return sql;
}

template<class T>
void BaseModel<T>::sqlUpdateSingle(const T &item, ModelFieldKeyHash &uniqueColumns) {
    std::string sql;

    for(const auto &[key, value]: item.getObjectValues()) {
        std::visit(
            [&item, &uniqueColumns, &key](const auto &arg) {
                std::string data;
                using Type = std::decay_t<decltype(arg)>;
                if constexpr(std::is_same_v<Type, std::chrono::system_clock::time_point>) {
                    data = timePointToString(arg);
                } else if constexpr(std::is_same_v<Type, std::string>) {
                    data = addExtraQuotes(arg);
                } else if constexpr(std::is_same_v<Type, int>) {
                    data = std::to_string(arg);
                } else if constexpr(std::is_same_v<Type, bool>) {
                    data = arg ? "true" : "false";
                } else if constexpr(std::is_same_v<Type, dec::decimal<2>>) {
                    std::stringstream ss;
                    ss << arg;
                    data = ss.str();
                } else if constexpr(std::is_same_v<Type, std::vector<std::string>>) {
                    data = "{";  // No single quotes here, we add them later when building SQL
                    for(const auto &str: arg) {
                        data.append(addExtraQuotes(str)).append(",");
                    }
                    if(!arg.empty()) {
                        data.pop_back();  // Remove trailing comma
                    }
                    data.append("}");  // End array representation
                } else {
                    data = arg;
                }
                if(data != "Null") {
                    uniqueColumns[key.getFieldName()].append(
                        fmt::format(R"( WHEN {} = {} THEN '{}' )", T::Field::id.getFullFieldName(), item.id, data));
                } else {
                    uniqueColumns[key.getFieldName()].append(
                        fmt::format(R"( WHEN {} = {} THEN {} )", T::Field::id.getFullFieldName(), item.id, data));
                }
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
    std::string sql = fmt::format(R"(UPDATE "{}" SET )", T::tableName);
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

    sql.append(fmt::format(" WHERE {} IN ({}) ", T::Field::id.getFullFieldName(), ids));
    sql.append(fmt::format(" RETURNING json_build_object({});", fieldsJsonObject()));
    return sql;
}

template<class T>
std::string BaseModel<T>::fieldsToString() {
    std::stringstream ss;
    for(const auto &[key, value]: T().getObjectValues()) {
        ss << key.getFieldName() << ", ";
    }
    return ss.str().substr(0, ss.str().size() - 2);
}

template<class T>
std::vector<BaseField> BaseModel<T>::allSetFields() const {
    std::string str;
    const typename T::Field field;
    std::vector<BaseField> fields;
    fields.reserve(field.allFields.size());
    for(const auto &fieldNames = field.allFields; const auto &[fieldName, baseField]: fieldNames) {
        fields.emplace_back(baseField);
    }
    return fields;
}

template<class T>
bool BaseModel<T>::fieldExists(const std::string &fieldName) const {
    const typename T::Field field;
    return field.allFields.find(fieldName) != field.allFields.end();
}

template<class T>
std::string
BaseModel<T>::sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params) {
    QuerySet qsCount = T().qsCount();
    QuerySet qsPage = T().qsPage(page, limit);

    typename T::Field field;
    auto orderIt = params.find("order");
    auto orderField = orderIt != params.end() && fieldExists(orderIt->second) ? field.allFields[orderIt->second]
                                                                              : T::Field::updatedAt;

    auto directionIt = params.find("direction");
    bool isAsc = directionIt != params.end() && directionIt->second == "asc";

    QuerySet qs(T::tableName, limit, "data");
    qs.offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
        .only(allSetFields())
        .order_by(std::make_pair(orderField, isAsc), std::make_pair(T::Field::id, false));
    applyFilters(qs, qsCount, params);
    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}

template<class T>
QuerySet BaseModel<T>::qsCount() {
    QuerySet qsCount(T::tableName, "total", false, true);
    return std::move(qsCount.functions(Function("count(*)::integer")));
}

template<class T>
QuerySet BaseModel<T>::qsPage(int page, int limit) {
    QuerySet qsCount = T().qsCount();
    QuerySet qsPage(ItemModel::tableName, "_page", false, true);
    return std::move(qsPage.functions(
        Function(fmt::format("GetValidPage({}, {}, (SELECT * FROM {}))", page, limit, qsCount.alias()))));
}

template<class T>
std::string BaseModel<T>::fieldsJsonObject() {
    std::string str;
    const typename T::Field field;
    for(const auto &fieldNames = field.allFields; const auto &[fieldName, baseField]: fieldNames) {
        str += fmt::format("'{}', {}, ", fieldName, baseField.getFullFieldName());
    }
    return str.substr(0, str.size() - 2);
}

template<class T>
std::string BaseModel<T>::sqlSelectOne(const std::string &field,
                                       const std::string &value,
                                       [[maybe_unused]] const std::map<std::string, std::string, std::less<>> &params) {
    QuerySet qs(T::tableName, T::tableName, true);
    qs.jsonFields(addExtraQuotes(fieldsJsonObject())).filter(field, std::string(value));
    return qs.buildSelect();
}

template<class T>
std::map<std::string, std::pair<std::string, std::string>, std::less<>> BaseModel<T>::joinMap() const {
    return {};
}

template<class T>
void BaseModel<T>::applyFilters(QuerySet &qs,
                                QuerySet &qsCount,
                                const std::map<std::string, std::string, std::less<>> &params) const {
    typename T::Field field;
    for(const auto &[key, value]: params) {
        if(fieldExists(key)) {
            qs.filter(field.allFields[key].getFullFieldName(), value);
            qsCount.filter(field.allFields[key].getFullFieldName(), value);
        }
    }
}

template class api::v1::BaseModel<PageModel>;
template class api::v1::BaseModel<ItemModel>;
template class api::v1::BaseModel<UserModel>;
template class api::v1::BaseModel<MediaModel>;
template class api::v1::BaseModel<ShippingProfileModel>;
template class api::v1::BaseModel<ShippingRateModel>;
template class api::v1::BaseModel<CountryModel>;
template class api::v1::BaseModel<CountriesIpsModel>;
template class api::v1::BaseModel<OrderModel>;
template class api::v1::BaseModel<BasketItemModel>;
template class api::v1::BaseModel<BasketModel>;
template class api::v1::BaseModel<AddressModel>;
template class api::v1::BaseModel<ReviewModel>;
template class api::v1::BaseModel<FinancialDetailsModel>;
template class api::v1::BaseModel<SocialMediaModel>;
