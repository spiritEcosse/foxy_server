#pragma once

#include <string>
#include <utility>
#include <vector>
#include <variant>
#include "StringUtils.h"
#include "BaseField.h"
#include "Function.h"
#include <fmt/core.h>

namespace api::v1 {
    using FieldOrFunction = std::variant<std::reference_wrapper<const BaseField>, Function>;

    namespace detail {
        template<typename T>
        std::string getFullFieldName(const T &obj);

        template<>
        inline std::string getFullFieldName(const Function &obj) {
            return obj.getFullFieldName();
        }

        template<>
        inline std::string getFullFieldName(const std::reference_wrapper<const BaseField> &obj) {
            return obj.get().getFullFieldName();
        }

        template<>
        inline std::string
        getFullFieldName(const std::variant<std::reference_wrapper<const BaseField>, Function> &obj) {
            return std::visit(
                [](const auto &value) {
                    return getFullFieldName(value);
                },
                obj);
        }
    }

    struct JoinInfo {
        std::vector<std::pair<std::string, std::string>> joinTable;
        std::vector<std::string> joinCondition;
        std::vector<std::pair<std::string, std::string>> leftJoinTable;
        std::vector<std::string> leftJoinCondition;
    };

    struct FilterInfo {
        std::vector<std::tuple<std::string, std::string, std::string, std::string, bool>> filters;
        std::vector<std::tuple<std::string, std::string, std::string, std::string, bool>> groupFilters;
    };

    struct OrderInfo {
        std::vector<std::pair<std::variant<std::reference_wrapper<const BaseField>, Function>, bool>> orderFields;
        std::string orderBy;
    };

    struct DistinctInfo {
        std::vector<std::reference_wrapper<const BaseField>> distinctFields;
        std::string distinctOn;
    };

    class BaseQuerySet : public BaseClass {
    public:
        using BaseClass::BaseClass;

        [[nodiscard]] std::string buildSelectOne() const {
            std::string sql = "SELECT ";
            if(_doAndCheck) {
                sql += " do_and_check('SELECT ";
            }
            if(_jsonFields.empty()) {
                sql += buildOnlyFields();
                sql += buildFunctions();
            } else {
                sql += fmt::format(" json_build_object({}) ", _jsonFields);
            }
            sql += fmt::format(" FROM \"{}\" ", tableName);
            sql += generateJoinSQL(joinInfo.joinTable, joinInfo.joinCondition, "INNER");
            sql += generateJoinSQL(joinInfo.leftJoinTable, joinInfo.leftJoinCondition, "LEFT");
            sql += filter_impl();
            sql += " LIMIT 1 ";
            if(_doAndCheck) {
                sql += "')";
            }
            return sql;
        }

        [[nodiscard]] std::string buildSelect() const {
            if(_one) {
                return buildSelectOne();
            }
            std::string sql;
            sql += " SELECT ";

            if(!distinctInfo.distinctFields.empty()) {
                sql += "DISTINCT ON (";
                for(const auto &field: distinctInfo.distinctFields) {
                    sql += field.get().getFullFieldName() + ",";
                }
                sql.pop_back();
                sql += ") ";
            }
            sql += buildOnlyFields();
            sql += buildFunctions();
            sql += fmt::format(" FROM \"{}\" ", tableName);
            sql += generateJoinSQL(joinInfo.joinTable, joinInfo.joinCondition, "INNER");
            sql += generateJoinSQL(joinInfo.leftJoinTable, joinInfo.leftJoinCondition, "LEFT");
            sql += filter_impl();
            sql += generateGroupBySQL(groupByFields);
            if(!orderInfo.orderFields.empty()) {
                sql += " ORDER BY ";
                for(const auto &[field, asc]: orderInfo.orderFields) {
                    sql += fmt::format("{} {}", detail::getFullFieldName(field), asc ? "ASC" : "DESC") + ",";
                }
                sql.pop_back();  // Remove the last comma
            }
            sql += limit_impl();
            return sql;
        }

        explicit BaseQuerySet(const std::string_view &tableName,
                              int limit,
                              std::string alias,
                              bool returnInMain = true) :
            tableName(tableName), _limit(limit), _alias(std::move(alias)), _doAndCheck(false),
            _returnInMain(returnInMain) {}

        explicit BaseQuerySet(const std::string_view &tableName,
                              const std::string_view &alias,
                              bool doAndCheck = false,
                              bool returnInMain = true) :
            tableName(tableName), _alias(alias), _one(true), _doAndCheck(doAndCheck), _returnInMain(returnInMain) {}

        std::string tableName;
        FilterInfo filterInfo;
        JoinInfo joinInfo;
        OrderInfo orderInfo;
        DistinctInfo distinctInfo;
        std::string _jsonFields;
        std::vector<std::reference_wrapper<const BaseField>> onlyFields;
        std::vector<Function> functionsSet;
        std::vector<std::reference_wrapper<const BaseField>> groupByFields;
        int _limit{};
        std::string _offset;
        std::string _alias;
        bool _one{};
        bool _doAndCheck{};
        bool _returnInMain{};

        template<typename T>
        void group_by_impl(const T &t) {
            groupByFields.emplace_back(t);
        }

        template<typename T, typename... Args>
        void group_by_impl(const T &t, Args... args) {
            groupByFields.emplace_back(t);
            group_by_impl(args...);
        }

        [[nodiscard]] std::string aliasQueryMain() const {
            std::string query;
            if(!_returnInMain) {
                return query;
            }
            if(_one) {
                query += fmt::format("'{0}', (SELECT * FROM {0} ),", _alias);
            } else {
                query += fmt::format("'{0}', COALESCE((SELECT json_agg({0}.*) FROM ( SELECT * FROM {0} "
                                     ") as {0}), '[]'),",
                                     _alias);
            }
            return query;
        }

        [[nodiscard]] std::string aliasQuery() const {
            return fmt::format(" {} AS ( {} ),", _alias, buildSelect());
        }

        [[nodiscard]] static std::string removeLastComma(const std::string_view &query) {
            return std::string(query.substr(0, query.size() - 2));  // remove last comma and space
        }

        [[nodiscard]] std::string buildOnlyFields() const {
            std::string sql;

            for(const auto &field: onlyFields) {
                sql += fmt::format("{}, ", field.get().getFullFieldName());
            }
            return removeLastComma(sql);
        }

        template<typename T>
        void only_impl(const T &t) {
            onlyFields.emplace_back(t);
        }

        template<typename T, typename... Args>
        void only_impl(const T &t, Args... args) {
            onlyFields.emplace_back(t);
            only_impl(args...);
        }

        template<typename T>
        void distinct_impl(const T &t) {
            distinctInfo.distinctFields.emplace_back(t);
        }

        template<typename T, typename... Args>
        void distinct_impl(const T &t, const Args &...args) {
            distinctInfo.distinctFields.emplace_back(t);
            distinct_impl(args...);
        }

        // Base case: no arguments left
        static std::string addQuery_impl() {
            return "";
        }

        template<typename T, typename... Args>
        static std::string addQuery_impl(T &&t, Args &&...args) {
            return fmt::format("{} {}", std::forward<T>(t).aliasQuery(), addQuery_impl(std::forward<Args>(args)...));
        }

        // Base case: no arguments left
        static std::string addQueryMain_impl() {
            return "";
        }

        template<typename T, typename... Args>
        static std::string addQueryMain_impl(T &&t, Args &&...args) {
            return fmt::format("{} {}",
                               std::forward<T>(t).aliasQueryMain(),
                               addQueryMain_impl(std::forward<Args>(args)...));
        }

        template<typename T>
        void filter_impl(T t) {
            std::string field = std::get<0>(t);
            std::string op = std::get<1>(t);
            std::string value = std::get<2>(t);
            bool escape = std::get<3>(t);
            std::string conjunction = std::get<4>(t);
            filterInfo.filters.emplace_back(field, op, value, conjunction, escape);
        }

        template<typename T, typename... Args>
        void filter_impl(T t, Args... args) {
            std::string field = std::get<0>(t);
            std::string op = std::get<1>(t);
            std::string value = std::get<2>(t);
            bool escape = std::get<3>(t);
            std::string conjunction = std::get<4>(t);
            filterInfo.filters.emplace_back(field, op, value, conjunction, escape);
            filter_impl(args...);
        }

        template<typename T>
        void order_by_impl(const T &t) {
            orderInfo.orderFields.emplace_back(t.first, t.second);
        }

        template<typename T, typename... Args>
        void order_by_impl(const T &t, const Args &...args) {
            orderInfo.orderFields.emplace_back(t.first, t.second);
            // Recursively call order_by_impl with the rest of the arguments
            order_by_impl(args...);
        }

        template<typename T>
        void functions_impl(T &&t) {
            functionsSet.emplace_back(std::forward<T>(t));
        }

        template<typename T, typename... Args>
        void functions_impl(T &&t, Args &&...args) {
            functionsSet.emplace_back(std::forward<T>(t));
            functions_impl(std::forward<Args>(args)...);
        }

        static std::string generateJoinSQL(const std::vector<std::pair<std::string, std::string>> &tables,
                                           const std::vector<std::string> &conditions,
                                           const std::string &joinType) {
            std::string sql;
            for(size_t i = 0; i < tables.size(); ++i) {
                auto [tableName, alias] = tables[i];
                if(!tableName.empty()) {
                    sql += fmt::format(R"( {} JOIN "{}" {} ON {})",
                                       joinType,
                                       tableName,
                                       alias.empty() ? "" : fmt::format(R"( AS "{}")", alias),
                                       conditions[i]);
                }
            }
            return sql;
        }

        static std::string generateGroupBySQL(const std::vector<std::reference_wrapper<const BaseField>> &fields) {
            std::string sql;
            if(!fields.empty()) {
                sql += " GROUP BY ";
                for(const auto &field: fields) {
                    sql += fmt::format("{}, ", field.get().getFullFieldName());
                }
            }
            return removeLastComma(sql);
        }

        [[nodiscard]] std::string buildFunctions() const {
            std::string sql;
            if(!orderInfo.orderFields.empty() && !functionsSet.empty()) {
                sql += ", ";
            }
            for(const auto &function: functionsSet) {
                sql += fmt::format("{}, ", function.toStr());
            }
            return removeLastComma(sql);
        }

        [[nodiscard]] std::string limit_impl() const {
            std::string sql;
            if(_limit) {
                sql += fmt::format(" LIMIT {}", std::to_string(_limit));
                if(!_offset.empty()) {
                    sql += fmt::format(" OFFSET {}", _offset);
                }
            }
            return sql;
        }

        [[nodiscard]] std::string filter_impl() const {
            if(filterInfo.filters.empty()) {
                return "";
            }
            std::string query = " WHERE ";

            if(!filterInfo.groupFilters.empty())
                query += " ( ";
            for(const auto &[field, op, value, conjunction, escape]: filterInfo.groupFilters) {
                if(escape) {
                    query += fmt::format(" {} {} '{}' {} ", field, op, value, conjunction);
                } else {
                    query += fmt::format(" {} {} {} {} ", field, op, value, conjunction);
                }
            }
            if(!filterInfo.groupFilters.empty()) {
                query += " ) ";
                if(!filterInfo.filters.empty()) {
                    query += " AND ";
                }
            }
            for(const auto &[field, op, value, conjunction, escape]: filterInfo.filters) {
                if(escape) {
                    query += fmt::format(" {} {} '{}' {} ", field, op, value, conjunction);
                } else {
                    query += fmt::format(" {} {} {} {} ", field, op, value, conjunction);
                }
            }
            query = std::string(query.substr(0, query.size() - 4));

            if(_doAndCheck) {
                query = addExtraQuotes(query);
            }
            return query;
        }

        template<class T, bool isLeftJoin>
        void join_impl(const T &model, const std::string &alias, std::string &&addConditions) {
            // Get the last element from joinTable
            std::string lastJoinTable = joinInfo.joinTable.empty() ? tableName : joinInfo.joinTable.back().first;

            auto mapFields = model.joinMap();
            // Use lastJoinTable to find in model's joinMap
            auto it = mapFields.find(lastJoinTable);
            if(it == mapFields.end()) {
                it = mapFields.find(tableName);
            }
            if(it != mapFields.end()) {
                const auto &[joinFieldFirstTable, joinFieldSecondField] = it->second;
                auto &joinTable = isLeftJoin ? joinInfo.leftJoinTable : joinInfo.joinTable;
                auto &joinCondition = isLeftJoin ? joinInfo.leftJoinCondition : joinInfo.joinCondition;
                joinTable.emplace_back(std::move(model.tableName), alias);
                joinCondition.emplace_back(std::move(
                    fmt::format(R"({}.{} = {} {})",
                                alias.empty() ? joinFieldFirstTable.substr(0, joinFieldFirstTable.find('.')) : alias,
                                joinFieldFirstTable.substr(joinFieldFirstTable.find('.') + 1),
                                joinFieldSecondField,
                                std::move(addConditions))));
            }
        }
    };

    class QuerySet final : public BaseQuerySet {
    public:
        using BaseQuerySet::BaseQuerySet;

        QuerySet(const std::string_view &tableName,
                 const int limit,
                 std::string alias,
                 const bool returnInMain = true) : BaseQuerySet(tableName, limit, std::move(alias), returnInMain) {}

        QuerySet(const std::string_view &tableName,
                 const std::string_view &alias,
                 const bool doAndCheck = false,
                 const bool returnInMain = true) : BaseQuerySet(tableName, alias, doAndCheck, returnInMain) {}

        [[nodiscard]] std::string alias() const {
            return _alias;
        }

        template<typename... Args>
        QuerySet &group_by(const Args &...args) {
            group_by_impl(args...);
            return *this;
        }

        template<typename... Args>
        QuerySet &filter(Args... args) {
            filter_impl(args...);
            return *this;
        }

        QuerySet &filter(const std::string &field,
                         const std::string &value,
                         bool escape = true,
                         const std::string &op = "=",
                         const std::string &conjunction = "AND") {
            filterInfo.filters.emplace_back(field, op, value, conjunction, escape);
            return *this;
        }

        QuerySet &filter(std::string &&field,
                         std::string &&value,
                         bool escape = true,
                         std::string &&op = "=",
                         std::string &&conjunction = "AND") {
            filterInfo.filters.emplace_back(std::move(field),
                                            std::move(op),
                                            std::move(value),
                                            std::move(conjunction),
                                            escape);
            return *this;
        }

        QuerySet &
        filter(const std::vector<std::tuple<std::string, std::string, std::string, bool, std::string>> &conditions) {
            for(const auto &condition: conditions) {
                const auto &[field, op, value, escape, conjunction] = condition;
                filterInfo.groupFilters.emplace_back(field, op, value, conjunction, escape);
            }
            return *this;
        }

        template<typename... Args>
        QuerySet &order_by(const Args &...args) {
            order_by_impl(args...);
            return *this;
        }

        template<typename... Args>
        QuerySet &functions(Args &&...args) {
            functions_impl(std::forward<Args>(args)...);
            return *this;
        }

        template<class T>
        QuerySet &join(const T &model, const std::string &alias = "", std::string &&addConditions = "") {
            join_impl<T, false>(model, alias, std::move(addConditions));
            return *this;
        }

        template<class T>
        QuerySet &left_join(const T &model, const std::string &alias = "", std::string &&addConditions = "") {
            join_impl<T, true>(model, alias, std::move(addConditions));
            return *this;
        }

        template<typename... Args>
        QuerySet &distinct(const Args &...args) {
            distinct_impl(args...);
            return *this;
        }

        QuerySet &only(const std::vector<std::reference_wrapper<const BaseField>> &fields) {
            for(const auto &field: fields) {
                only(field);
            }
            return *this;
        }

        QuerySet &only(const std::reference_wrapper<const BaseField> &field) {
            onlyFields.emplace_back(field);
            return *this;
        }

        template<typename... Args>
        QuerySet &only(const Args &...args) {
            only_impl(args...);
            return *this;
        }

        QuerySet &limit(int limit) {
            this->_limit = limit;
            return *this;
        }

        QuerySet &offset(std::string &&offset) {
            _offset = std::move(offset);
            return *this;
        }

        QuerySet &jsonFields(std::string &&jsonFields) {
            _jsonFields = std::move(jsonFields);
            return *this;
        }

        template<typename... Args>
        static std::string buildQuery(Args &&...args) {
            std::string query = removeLastComma(fmt::format("WITH {}", addQuery_impl(args...)));
            return fmt::format(" {} SELECT json_build_object({}) as result",
                               query,
                               removeLastComma(addQueryMain_impl(std::forward<Args>(args)...)));
        }
    };
}
