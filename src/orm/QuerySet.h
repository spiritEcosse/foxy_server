#include <string>
#include <vector>
#include <map>

class QuerySet {
private:
    std::string tableName;
    std::vector<std::pair<std::string, std::string>> filters;
    std::vector<std::pair<std::string, bool>> orderFields;
    std::vector<std::string> onlyFields;
    std::string joinTable;
    std::string joinCondition;
    std::vector<std::string> distinctFields;
    std::string leftJoinTable;
    std::string leftJoinCondition;
    std::string distinctOn;
    std::string orderBy;
    int _limit;
    int _page;

public:
    explicit QuerySet(std::string tableName, int limit = 0, int page = 1) :
    tableName(std::move(tableName)), _limit(limit), _page(page) {}

    QuerySet& filter(std::vector<std::pair<std::string, std::string>> fields) {
        filters = std::move(fields);
        return *this;
    }

    QuerySet& order_by(std::vector<std::pair<std::string, bool>> fields) {
        orderFields = std::move(fields);
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

    QuerySet& distinct(std::vector<std::string> fields) {
        distinctFields = std::move(fields);
        return *this;
    }

    QuerySet& limit(int limit) {
        this->_limit = limit;
        return *this;
    }

    QuerySet& page(int page) {
        this->_page = page;
        return *this;
    }

    QuerySet& only(std::vector<std::string> fields) {
        onlyFields = std::move(fields);
        return *this;
    }

    std::string buildSelect() const {
        std::string query = "WITH items AS (";
        std::string sqlItems;
        sqlItems += "SELECT ";

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
        if (!filters.empty()) {
            sqlItems += " WHERE ";
            for (const auto& [field, value] : filters) {
                sqlItems += field + " = '" + value + "' AND ";
            }
            sqlItems = sqlItems.substr(0, sqlItems.size() - 5);  // Remove the last " AND "
        }
        if (!orderFields.empty()) {
            sqlItems += " ORDER BY ";
            for (const auto& [field, asc] : orderFields) {
                sqlItems += field + (asc ? " ASC" : " DESC") + ",";
            }
            sqlItems.pop_back();  // Remove the last comma
        }
        query += sqlItems;
        query += ") ";
        query += ", item_count AS ( ";
        query += "    SELECT count(*)::integer as count FROM items ";
        query += ") ";

        query += ", valid_page AS ( ";
        query += "    SELECT GetValidPage(" + std::to_string(_page) + ", " + std::to_string(_limit)
            + ", (SELECT count FROM item_count)) as page ";
        query += ") ";
        query += "SELECT ";

        query += "   (SELECT page FROM valid_page) as page, ";
        query += "   (SELECT count FROM item_count) as count, ";
        query += "   (SELECT json_agg(t.*) FROM ( ";
        query += sqlItems;
        if (_limit > 0) {
            query += " LIMIT " + std::to_string(_limit);
        }
        if (_page > 0) {
            query += " OFFSET ((SELECT page FROM valid_page) - 1) * " + std::to_string(_limit);
        }
        query += "    ) as t ";
        query += ") as items;";
        return query;
    }
};