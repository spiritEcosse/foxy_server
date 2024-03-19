#include <string>
#include <vector>
#include <map>
#include "src/utils/db/String.h"
#include <fmt/core.h>

class QuerySet {
private:
    std::string tableName;
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> filters;
    std::vector<std::pair<std::string, bool>> orderFields;
    std::vector<std::string> onlyFields;
    std::string joinTable;
    std::string joinCondition;
    std::vector<std::string> distinctFields;
    std::string leftJoinTable;
    std::string leftJoinCondition;
    std::string distinctOn;
    std::string orderBy;
    std::string _jsonFields;
    int _limit;
    int _page;
    bool _returnCount;
    bool _one;

public:
    explicit QuerySet(std::string tableName, bool one = false, int limit = 0, int page = 1, bool returnCount = false) :
    tableName(std::move(tableName)), _limit(limit), _page(page), _returnCount(returnCount), _one(one) {}

    template<typename... Args>
    QuerySet& filter(Args... args) {
        filter_impl(args...);
        return *this;
    }

    QuerySet& filter(std::string field, std::string value) {
        filters.emplace_back(field, "=", value, "AND");
        return *this;
    }

    template<typename... Args>
    QuerySet& or_filter(Args... args) {
        or_filter_impl(args...);
        return *this;
    }

    QuerySet& or_filter(std::string field, std::string value) {
        filters.emplace_back(field, "=", value, "OR");
        return *this;
    }

    template<typename... Args>
    QuerySet& order_by(Args... args) {
        order_by_impl(args...);
        return *this;
    }

    QuerySet& join(std::string table, std::string condition) {
        joinTable = std::move(table);
        joinCondition = std::move(condition);
        return *this;
    }

    QuerySet& left_join(std::string table, std::string condition) {
        leftJoinTable = std::move(table);
        leftJoinCondition = std::move(condition);
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

    QuerySet& jsonFields(std::string jsonFields) {
        this->_jsonFields = std::move(jsonFields);
        return *this;
    }

    QuerySet& only(std::vector<std::string> fields) {
        onlyFields = std::move(fields);
        return *this;
    }

    QuerySet& returnCount(bool returnCount = true) {
        _returnCount = returnCount;
        return *this;
    }

    [[nodiscard]] std::string filter() const {
        if (filters.empty()) {
            return "";
        }
        std::string query = " WHERE ";
        for (const auto& [field, op, value, conjunction] : filters) {
            if (value == "NULL") {
                query += fmt::format("{} {} {} {} ", field, op, value, conjunction);
            } else {
                query += fmt::format("{} {} '{}' {} ", field, op, value, conjunction);
            }
        }
        return query.substr(0, query.size() - 4);  // Remove the last " AND " or " OR "
    }

    template<typename... Args>
    std::string addQuery(Args... args) {
        return buildSelect() + addQuery_impl(args...);
    }

    [[nodiscard]] std::string buildSelect() const {
        if (_one) {
            return buildSelectOne();
        }
        std::string query = "WITH items AS (";
        std::string sqlItems;
        sqlItems += " SELECT ";

        if (!distinctFields.empty()) {
            sqlItems += "DISTINCT ON (";
            for (const auto& field : distinctFields) {
                sqlItems += field + ",";
            }
            sqlItems.pop_back();
            sqlItems += ") ";
        }
        if (!onlyFields.empty()) {
            for (const auto& field : onlyFields) {
                sqlItems += field + ",";
            }
            sqlItems.pop_back();
        } else {
            sqlItems += "*";
        }
        sqlItems += " FROM " + tableName;
        if (!joinTable.empty()) {
            sqlItems += " INNER JOIN " + joinTable + " ON " + joinCondition;
        }
        if (!leftJoinTable.empty()) {
            sqlItems += " LEFT JOIN " + leftJoinTable + " ON " + leftJoinCondition;
        }
        sqlItems += filter();
        if (!orderFields.empty()) {
            sqlItems += " ORDER BY ";
            for (const auto& [field, asc] : orderFields) {
                sqlItems += field + (asc ? " ASC" : " DESC") + ",";
            }
            sqlItems.pop_back();  // Remove the last comma
        }
        query += sqlItems;
        query += ") ";
        if (_returnCount) {
            query += ", item_count AS ( ";
            query += "    SELECT count(*)::integer as count FROM items ";
            query += ") ";
            query += ", valid_page AS ( ";
            query += "    SELECT GetValidPage(";
            query += std::to_string(_page);
            query += ", ";
            query += std::to_string(_limit);
            query += ", (SELECT count FROM item_count)) as page ";
            query += ") ";
        }
        query += " SELECT ";

        if (_returnCount) {
            query += "   (SELECT page FROM valid_page) as page, ";
            query += "   (SELECT count FROM item_count) as count, ";
        }
        query += "   (SELECT json_agg(t.*) FROM ( ";
        if (!_limit) {
            query += "        SELECT * FROM items ";
        } else {
            query += sqlItems;
            query += " LIMIT ";
            query += std::to_string(_limit);
            query += " OFFSET ((SELECT page FROM valid_page) - 1) * ";
            query += std::to_string(_limit);
        }
        query += "    ) as t ";
        query += ") as items";
        return query;
    }

    [[nodiscard]] std::string buildSelectOne() const {
        std::string query = "SELECT do_and_check(\'SELECT json_build_object(";
        query += _jsonFields;
        query += ") FROM " + tableName;
        query += addExtraQuotes(filter());
        query += "\') as " + tableName;
        return query;
    }

private:
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
    std::string addQuery_impl(T t, Args... args) {
        // Process the first argument here
        std::string query = t.buildSelect();
        if (size_t pos = query.find("SELECT"); pos == 0) { // Check if "SELECT" is at the start of the string
            // Erase "SELECT " (7 characters)
            query.erase(pos, 7);
            query = ", " + query;
        }
        // Recursively call addQuery_impl with the rest of the arguments
        query += addQuery_impl(args...);
        return query;
    }

    template<typename T>
    void filter_impl(T t) {
        std::string field = std::get<0>(t);
        std::string op = std::get<1>(t);
        std::string value = std::get<2>(t);
        filters.emplace_back(field, op, value, "AND");
    }

    template<typename T, typename... Args>
    void filter_impl(T t, Args... args) {
        std::string field = std::get<0>(t);
        std::string op = std::get<1>(t);
        std::string value = std::get<2>(t);
        filters.emplace_back(field, op, value, "AND");
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

    template<typename T>
    void or_filter_impl(T t) {
        std::string field = std::get<0>(t);
        std::string op = std::get<1>(t);
        std::string value = std::get<2>(t);
        filters.emplace_back(field, op, value, "OR");
    }

    template<typename T, typename... Args>
    void or_filter_impl(T t, Args... args) {
        std::string field = std::get<0>(t);
        std::string op = std::get<1>(t);
        std::string value = std::get<2>(t);
        filters.emplace_back(field, op, value, "OR");
        or_filter_impl(args...);
    }
};