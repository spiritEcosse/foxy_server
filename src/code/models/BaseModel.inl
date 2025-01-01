#pragma once

namespace api::v1 {

    template<class T>
    BaseModel<T>::BaseModel(const Json::Value &json) {
        if(json.isMember(Field::id.getFieldName())) {
            id = json[Field::id.getFieldName()].asInt();
        }
    }

    template<class T>
    std::string BaseModel<T>::sqlDelete(int id) {
        return fmt::format(R"(DELETE FROM "{}" WHERE {} = {})", T::tableName, T::Field::id.getFullFieldName(), id);
    }

    template<class T>
    std::string BaseModel<T>::sqlDeleteMultiple(const std::vector<int> &ids) {
        auto sql = fmt::format(R"(DELETE FROM "{}" WHERE {} IN ( )", T::tableName, T::Field::id.getFullFieldName());

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
        std::string sql = fmt::format(R"(INSERT INTO "{}" ({}) VALUES )", T::tableName, fieldsToString());
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
                        uniqueColumns[key.get().getFieldName()].append(
                            fmt::format(R"( WHEN {} = {} THEN '{}' )", T::Field::id.getFullFieldName(), item.id, data));
                    } else {
                        uniqueColumns[key.get().getFieldName()].append(
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
            ss << key.get().getFieldName() << ", ";
        }
        return ss.str().substr(0, ss.str().size() - 2);
    }

    template<class T>
    std::vector<std::reference_wrapper<const BaseField>> BaseModel<T>::allSetFields() {
        const typename T::Field field;
        std::vector<std::reference_wrapper<const BaseField>> fields;
        fields.reserve(field.allFields.size());
        for(const auto &fieldNames = field.allFields; const auto &[fieldName, baseField]: fieldNames) {
            fields.emplace_back(baseField);
        }
        return fields;
    }

    template<class T>
    std::string
    BaseModel<T>::sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params) {
        QuerySet qsCount = T::qsCount();
        QuerySet qsPage = T::qsPage(page, limit);

        typename T::Field field;
        const auto orderIt = params.find("order");
        auto it = field.allFields.find(orderIt->second);
        const auto &orderField = (orderIt != params.end() && it != field.allFields.end())
                                     ? std::cref(field.allFields.at(orderIt->second))
                                     : std::cref(field.updatedAt);

        const auto directionIt = params.find("direction");
        bool isAsc = directionIt != params.end() && directionIt->second == "asc";

        QuerySet qs(T::tableName, limit, "data");
        qs.offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
            .only(allSetFields())
            .order_by(std::make_pair(orderField, isAsc), std::make_pair(std::cref(T::Field::id), false));
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
        const QuerySet qsCount = T::qsCount();
        QuerySet qsPage(T::tableName, "_page", false, true);
        return std::move(qsPage.functions(
            Function(fmt::format("GetValidPage({}, {}, (SELECT * FROM {}))", page, limit, qsCount.alias()))));
    }

    template<class T>
    std::string BaseModel<T>::fieldsJsonObject() {
        std::string str;
        const typename T::Field field;
        for(const auto &fieldNames = field.allFields; const auto &[fieldName, baseField]: fieldNames) {
            str += fmt::format("'{}', {}, ", fieldName, baseField.get().getFullFieldName());
        }
        return str.substr(0, str.size() - 2);
    }

    template<class T>
    std::string
    BaseModel<T>::sqlSelectOne(const std::string &field,
                               const std::string &value,
                               [[maybe_unused]] const std::map<std::string, std::string, std::less<>> &params) {
        QuerySet qs(T::tableName, T::tableName, true);
        qs.jsonFields(addExtraQuotes(fieldsJsonObject())).filter(field, std::string(value));
        return qs.buildSelect();
    }

    template<class T>
    void BaseModel<T>::applyFilters(QuerySet &qs,
                                    QuerySet &qsCount,
                                    const std::map<std::string, std::string, std::less<>> &params) {
        typename T::Field field;
        for(const auto &[key, value]: params) {
            auto it = field.allFields.find(key);
            if(it != field.allFields.end()) {
                qs.filter(it->second.get().getFullFieldName(), value);
                qsCount.filter(it->second.get().getFullFieldName(), value);
            }
        }
    }

}  // namespace api::v1