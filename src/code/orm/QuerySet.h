#pragma once

#include <string>
#include <utility>
#include <vector>
#include <variant>
#include "StringUtils.h"
#include "BaseField.h"
#include "Function.h"
#include "WhereClause.h"
#include <ranges>
#include <fmt/core.h>

namespace api::v1 {

    using FieldOrFunction = std::variant<const BaseField *, Function>;

    namespace detail {
        // Primary template
        template<typename T>
        std::string getFullFieldName(const T &obj) {
            return obj.getFullFieldName();
        }

        // Specialization for Function
        template<>
        inline std::string getFullFieldName(const Function &obj) {
            return obj.getFullFieldName();
        }

        // Specialization for BaseField pointer
        template<>
        inline std::string getFullFieldName(const BaseField *const &obj) {
            return obj->getFullFieldName();
        }

        // Variant specialization
        template<>
        inline std::string getFullFieldName(const std::variant<const BaseField *, Function> &obj) {
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

    struct OrderInfo {
        std::vector<std::pair<std::variant<const BaseField *, Function>, bool>> orderFields;
        std::string orderBy;
    };

    struct DistinctInfo {
        std::vector<const BaseField *> distinctFields;
        std::string distinctOn;
    };

    class BaseQuerySet : public BaseClass {
    public:
        using BaseClass::BaseClass;

        [[nodiscard]] std::string buildSelectOne() const {
            return fmt::format("{}{} FROM \"{}\" {}{} LIMIT 1{}",
                               _doAndCheck ? "SELECT do_and_check('SELECT " : "SELECT ",
                               _jsonFields.empty()
                                   ? fmt::format("{}{}", buildOnlyFields(), buildFunctions())
                                   : fmt::format("json_build_object({} {})", _jsonFields, buildFunctions()),
                               tableName,
                               generateJoinSQL(joinInfo.joinTable, joinInfo.joinCondition, "INNER"),
                               fmt::format("{}{}",
                                           generateJoinSQL(joinInfo.leftJoinTable, joinInfo.leftJoinCondition, "LEFT"),
                                           filter_impl()),
                               _doAndCheck ? "')" : "");
        }

        [[nodiscard]] std::string buildSelect() const {
            if(_one)
                return buildSelectOne();

            return fmt::format("{} SELECT {} {} {} FROM \"{}\" {} {} {} {} {} {}",
                               buildOtherQueries(),
                               createDistinctClause(),
                               buildOnlyFields(),
                               buildFunctions(),
                               tableName,
                               generateJoinSQL(joinInfo.joinTable, joinInfo.joinCondition, "INNER"),
                               generateJoinSQL(joinInfo.leftJoinTable, joinInfo.leftJoinCondition, "LEFT"),
                               filter_impl(),
                               generateGroupBySQL(),
                               buildOrderFields(),
                               limit_impl());
        }

        explicit BaseQuerySet(const std::string_view &tableName,
                              int limit,
                              std::string alias,
                              bool returnInMain = true,
                              const bool join = false) :
            tableName(tableName), _limit(limit), _alias(std::move(alias)), _returnInMain(returnInMain), _join(join) {}

        explicit BaseQuerySet(const std::string_view &tableName,
                              const std::string_view &alias,
                              bool doAndCheck = false,
                              bool returnInMain = true) :
            tableName(tableName), _alias(alias), _one(true), _doAndCheck(doAndCheck), _returnInMain(returnInMain) {}

        std::string tableName;
        std::vector<WhereClause> filters;
        JoinInfo joinInfo;
        OrderInfo orderInfo;
        DistinctInfo distinctInfo;
        std::string _jsonFields;
        std::vector<const BaseField *> onlyFields;
        std::vector<Function> functionsSet;
        std::vector<BaseQuerySet> otherQueries;
        std::vector<const BaseField *> groupByFields;
        int _limit{};
        int _offset{};
        std::string _alias;
        bool _one{};
        bool _doAndCheck{};
        bool _returnInMain{};
        bool _join{};

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
            return format("{}",
                          fmt::join(onlyFields | std::views::transform([](const auto &field) {
                                        return field->getFullFieldName();
                                    }),
                                    ", "));
        }

        [[nodiscard]] std::string buildOrderFields() const {
            if(orderInfo.orderFields.empty()) {
                return "";
            }
            return format(" ORDER BY {}",
                          fmt::join(orderInfo.orderFields | std::views::transform([](const auto &pair) {
                                        return fmt::format("{} {}",
                                                           detail::getFullFieldName(pair.first),
                                                           pair.second ? "ASC" : "DESC");
                                    }),
                                    ", "));
        }

        [[nodiscard]] std::string createDistinctClause() const {
            if(distinctInfo.distinctFields.empty()) {
                return "";
            }
            return format("DISTINCT ON ({}) ",
                          fmt::join(distinctInfo.distinctFields | std::views::transform([](const auto &field) {
                                        return field->getFullFieldName();
                                    }),
                                    ", "));
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

        [[nodiscard]] std::string generateGroupBySQL() const {
            if(groupByFields.empty()) {
                return "";
            }

            return format("GROUP BY {}",
                          fmt::join(groupByFields | std::views::transform([](const auto &field) {
                                        return field->getFullFieldName();
                                    }),
                                    ", "));
        }

        [[nodiscard]] std::string buildOtherQueries() const {
            if(otherQueries.empty()) {
                return "";
            }

            return format("WITH {}",
                          fmt::join(otherQueries | std::views::transform([](const auto &query) {
                                        return fmt::format("{} AS ({})", query.buildSelect(), query._alias);
                                    }),
                                    ", "));
        }

        [[nodiscard]] std::string buildFunctions() const {
            if(functionsSet.empty()) {
                return "";
            }

            return format("{}{}",
                          onlyFields.empty() && _jsonFields.empty() ? "" : ", ",
                          fmt::join(functionsSet | std::views::transform([](const auto &function) {
                                        return function.toStr();
                                    }),
                                    ", "));
        }

        [[nodiscard]] std::string limit_impl() const {
            if(!_limit) {
                return "";
            }

            return _offset ? fmt::format(" LIMIT {} OFFSET {}", _limit, _offset) : fmt::format(" LIMIT {}", _limit);
        }

        [[nodiscard]] std::string filter_impl() const {
            if(filters.empty()) {
                return "";
            }

            auto transformed = filters | std::views::transform([](const WhereClause &filter) {
                                   return filter.serialize();
                               });

            std::vector serialized(transformed.begin(), transformed.end());
            std::string query = fmt::format(" WHERE {} ", fmt::join(serialized, " "));
            return _doAndCheck ? addExtraQuotes(query) : query;
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
                 const bool returnInMain = true,
                 const bool join = false) : BaseQuerySet(tableName, limit, std::move(alias), returnInMain, join) {}

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

        QuerySet &filter(const BaseField *field, const std::string &value, const Operator op = Operator::EQUALS) {
            filters.emplace_back(field, value, op);
            return *this;
        }

        QuerySet &
        filter(const BaseField *field, const std::optional<bool> value, const Operator op = Operator::EQUALS) {
            filters.emplace_back(field, value, op);
            return *this;
        }

        QuerySet &filter(const BaseField *field1, const BaseField *field2) {
            filters.emplace_back(field1, field2);
            return *this;
        }

        QuerySet &filter(WhereClause whereClause) {
            filters.push_back(std::move(whereClause));
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

        QuerySet &only(const std::vector<const BaseField *> &fields) {
            for(const auto &field: fields) {
                only(field);
            }
            return *this;
        }

        QuerySet &only(const BaseField *field) {
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

        QuerySet &count(const BaseField *field) {
            functions(Function(fmt::format("count({})::integer", field->getFullFieldName())));
            return *this;
        }

        template<typename... Args>
        static std::string buildQuery(Args &&...args) {
            std::string query = removeLastComma(fmt::format("WITH {}", addQuery_impl(args...)));
            return fmt::format(" {} SELECT json_build_object({}) as result",
                               query,
                               removeLastComma(addQueryMain_impl(std::forward<Args>(args)...)));
        }

        [[nodiscard]] QuerySet &addDynamoDbCte(QuerySet cte) {
            otherQueries.push_back(std::move(cte));
            return *this;
        }
    };
}
