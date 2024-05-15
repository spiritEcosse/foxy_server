#pragma once

#include <string>
#include <utility>
#include <vector>
#include <map>
#include "src/utils/db/String.h"
#include <fmt/core.h>

class QuerySet {
private:
    std::string tableName;
    std::vector<std::tuple<std::string, std::string, std::string, std::string, bool>> filters;
    std::vector<std::tuple<std::string, std::string, std::string, std::string, bool>> groupFilters;
    std::vector<std::pair<std::string, bool>> orderFields;
    std::vector<std::string> onlyFields;
    std::vector<std::string> joinTable;
    std::vector<std::string> joinCondition;
    std::vector<std::string> leftJoinTable;
    std::vector<std::string> leftJoinCondition;
    std::vector<std::string> distinctFields;
    std::vector<std::string> groupByFields;
    std::string distinctOn;
    std::string orderBy;
    std::string _jsonFields;
    int _limit{};
    bool _one{};
    bool _doAndCheck{};
    bool _returnInMain{};
    std::string _offset;
    std::string _alias;

public:
    QuerySet() = default;
    QuerySet(const QuerySet&) = delete;  // Copy constructor
    QuerySet& operator=(const QuerySet&) = delete;  // Copy assignment operator
    QuerySet(QuerySet&&) noexcept = default;  // Move constructor
    QuerySet& operator=(QuerySet&&) noexcept = default;  // Move assignment operator
    virtual ~QuerySet() = default;

    explicit QuerySet(std::string tableName, int limit, std::string alias, bool returnInMain = true) :
        tableName(std::move(tableName)), _limit(limit), _alias(std::move(alias)), _returnInMain(returnInMain),
        _doAndCheck(false) {}

    explicit QuerySet(std::string tableName,
                      std::string alias,
                      bool doAndCheck = false,
                      bool returnInMain = true) :
        tableName(std::move(tableName)),
        _one(true), _doAndCheck(doAndCheck), _alias(std::move(alias)), _returnInMain(returnInMain) {}

    std::string alias() {
        return _alias;
    }

    QuerySet& group_by(std::string field) {
        groupByFields.push_back(std::move(field));
        return *this;
    }

    template<typename... Args>
    QuerySet& filter(Args... args) {
        filter_impl(args...);
        return *this;
    }

    QuerySet& filter(const std::string& field,
                     std::string value,
                     bool escape = true,
                     std::string op = "=",
                     std::string conjunction = "") {
        filters.emplace_back(field, op, value, conjunction, escape);
        return *this;
    }

    QuerySet&
    filter(const std::vector<std::tuple<std::string, std::string, std::string, bool, std::string>>& conditions) {
        for(const auto& condition: conditions) {
            const auto& [field, op, value, escape, conjunction] = condition;
            groupFilters.emplace_back(field, op, value, conjunction, escape);
        }
        return *this;
    }

    template<typename... Args>
    QuerySet& order_by(Args... args) {
        order_by_impl(args...);
        return *this;
    }

    QuerySet& join(std::string table, std::string condition) {
        joinTable.push_back(std::move(table));
        joinCondition.push_back(std::move(condition));
        return *this;
    }

    QuerySet& left_join(std::string table, std::string condition) {
        leftJoinTable.push_back(std::move(table));
        leftJoinCondition.push_back(std::move(condition));
        return *this;
    }

    template<typename... Args>
    QuerySet& distinct(Args... args) {
        distinct_impl(args...);
        return *this;
    }

    QuerySet& limit(int limit) {
        this->_limit = limit;
        return *this;
    }

    QuerySet& offset(std::string offset) {
        _offset = std::move(offset);
        return *this;
    }

    QuerySet& jsonFields(std::string jsonFields) {
        this->_jsonFields = std::move(jsonFields);
        return *this;
    }

    QuerySet& only(std::vector<std::string> fields) {
        onlyFields = std::move(fields);
        return *this;
    }

    [[nodiscard]] std::string filter() const {
        if(filters.empty()) {
            return "";
        }
        std::string query = " WHERE ";

        if(!groupFilters.empty())
            query += " ( ";
        for(const auto& [field, op, value, conjunction, escape]: groupFilters) {
            if(escape) {
                query += fmt::format(" {} {} '{}' {} ", field, op, value, conjunction);
            } else {
                query += fmt::format(" {} {} {} {} ", field, op, value, conjunction);
            }
        }
        if(!groupFilters.empty()) {
            query += " ) ";
            if(!filters.empty()) {
                query += " AND ";
            }
        }
        for(const auto& [field, op, value, conjunction, escape]: filters) {
            if(escape) {
                query += fmt::format(" {} {} '{}' {} ", field, op, value, conjunction);
            } else {
                query += fmt::format(" {} {} {} {} ", field, op, value, conjunction);
            }
        }

        if(_doAndCheck) {
            query = addExtraQuotes(query);
        }
        return query;
    }

    template<typename... Args>
    static std::string buildQuery(Args&&... args) {
        std::string query = removeLastComma(fmt::format("WITH {}", addQuery_impl(args...)));
        return fmt::format(" {} SELECT json_build_object({}) as result",
                           query,
                           removeLastComma(addQueryMain_impl(args...)));
    }

    [[nodiscard]] std::string buildSelect() const {
        if(_one) {
            return buildSelectOne();
        }
        std::string sql;
        sql += " SELECT ";

        if(!distinctFields.empty()) {
            sql += "DISTINCT ON (";
            for(const auto& field: distinctFields) {
                sql += field + ",";
            }
            sql.pop_back();
            sql += ") ";
        }
        sql += buildOnlyFields();
        sql += fmt::format(" FROM \"{}\" ", tableName);
        sql += generateJoinSQL(joinTable, joinCondition, "INNER");
        sql += generateJoinSQL(leftJoinTable, leftJoinCondition, "LEFT");
        sql += filter();
        if(!orderFields.empty()) {
            sql += " ORDER BY ";
            for(const auto& [field, asc]: orderFields) {
                sql += field + (asc ? " ASC" : " DESC") + ",";
            }
            sql.pop_back();  // Remove the last comma
        }
        sql += limit();
        return sql;
    }

    [[nodiscard]] std::string buildSelectOne() const {
        std::string sql = "SELECT ";
        if(_doAndCheck) {
            sql += " do_and_check('SELECT ";
        }
        if(_jsonFields.empty()) {
            sql += buildOnlyFields();
        } else {
            sql += fmt::format(" json_build_object({}) ", _jsonFields);
        }
        sql += fmt::format(" FROM \"{}\" ", tableName);
        sql += generateJoinSQL(joinTable, joinCondition, "INNER");
        sql += generateJoinSQL(leftJoinTable, leftJoinCondition, "LEFT");
        sql += filter();
        sql += " LIMIT 1 ";
        if(_doAndCheck) {
            sql += "')";
        }
        return sql;
    }

private:
    [[nodiscard]] std::string aliasQueryMain() const {
        std::string query;
        if(!_returnInMain) {
            return query;
        }
        if(_one) {
            query += fmt::format("'{0}', (SELECT * FROM {0} ),", _alias);
        } else {
            query += fmt::format("'{0}', (SELECT json_agg({0}.*) FROM ( SELECT * FROM {0} ) as {0}),", _alias);
        }
        return query;
    }

    [[nodiscard]] std::string aliasQuery() const {
        return fmt::format(" {} AS ( {} ),", _alias, buildSelect());
    }

    [[nodiscard]] static std::string removeLastComma(const std::string& query) {
        return query.substr(0, query.size() - 2);  // remove last comma and space
    }

    [[nodiscard]] std::string buildOnlyFields() const {
        std::string onlyFieldsString;

        if(!onlyFields.empty()) {
            for(const auto& field: onlyFields) {
                onlyFieldsString += fmt::format("{},", field);
            }
            onlyFieldsString.pop_back();
        } else {
            onlyFieldsString += "*";
        }
        return onlyFieldsString;
    }

    template<typename T>
    void distinct_impl(T t) {
        std::string field = t;
        distinctFields.push_back(field);
    }

    template<typename T, typename... Args>
    void distinct_impl(T t, Args... args) {
        std::string field = t;
        distinctFields.push_back(field);
        distinct_impl(args...);
    }

    // Base case: no arguments left
    static std::string addQuery_impl() {
        return "";
    }

    template<typename T, typename... Args>
    static std::string addQuery_impl(T&& t, Args&&... args) {
        return fmt::format("{} {}", t.aliasQuery(), addQuery_impl(args...));
    }

    // Base case: no arguments left
    static std::string addQueryMain_impl() {
        return "";
    }

    template<typename T, typename... Args>
    static std::string addQueryMain_impl(T&& t, Args&&... args) {
        return fmt::format("{} {}", t.aliasQueryMain(), addQueryMain_impl(args...));
    }

    template<typename T>
    void filter_impl(T t) {
        std::string field = std::get<0>(t);
        std::string op = std::get<1>(t);
        std::string value = std::get<2>(t);
        bool escape = std::get<3>(t);
        std::string conjunction = std::get<4>(t);
        filters.emplace_back(field, op, value, conjunction, escape);
    }

    template<typename T, typename... Args>
    void filter_impl(T t, Args... args) {
        std::string field = std::get<0>(t);
        std::string op = std::get<1>(t);
        std::string value = std::get<2>(t);
        bool escape = std::get<3>(t);
        std::string conjunction = std::get<4>(t);
        filters.emplace_back(field, op, value, conjunction, escape);
        filter_impl(args...);
    }

    template<typename T>
    void order_by_impl(T t) {
        // Process the single argument here
        std::string field = t.first;
        bool ascending = t.second;
        // Add field to order_by_fields with the correct order
        orderFields.emplace_back(field, ascending);
    }

    template<typename T, typename... Args>
    void order_by_impl(T t, Args... args) {
        // Process the first argument here
        std::string field = t.first;
        bool ascending = t.second;
        // Add field to order_by_fields with the correct order
        orderFields.emplace_back(field, ascending);
        // Recursively call order_by_impl with the rest of the arguments
        order_by_impl(args...);
    }

    static std::string generateJoinSQL(const std::vector<std::string>& tables,
                                       const std::vector<std::string>& conditions,
                                       const std::string& joinType) {
        std::string sql;
        for(size_t i = 0; i < tables.size(); ++i) {
            if(!tables[i].empty()) {
                sql += fmt::format(" {} JOIN {} ON {}", joinType, tables[i], conditions[i]);
            }
        }
        return sql;
    }

    static std::string generateGroupBySQL(const std::vector<std::string>& fields) {
        std::string sql;
        if(!fields.empty()) {
            sql += " GROUP BY ";
            for(const auto& field: fields) {
                sql += field + ", ";
            }
            sql = sql.substr(0, sql.size() - 2);  // Remove the last comma and space
        }
        return sql;
    }

    [[nodiscard]] std::string limit() const {
        std::string sql;
        if(_limit) {
            sql += fmt::format(" LIMIT {}", std::to_string(_limit));
            if(!_offset.empty()) {
                sql += fmt::format(" OFFSET {}", _offset);
            }
        }
        return sql;
    }
};