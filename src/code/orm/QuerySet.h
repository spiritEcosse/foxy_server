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

    [[nodiscard]] static std::string removeLastComma(const std::string_view &query) {
        return std::string(query.substr(0, query.size() - 2));  // remove last comma and space
    }

    namespace detail {
        template<typename T>
        std::string getFullFieldName(const T &obj) {
            return obj.getFullFieldName();
        }

        template<>
        inline std::string getFullFieldName(const Function &obj) {
            return obj.getFullFieldName();
        }

        template<>
        inline std::string getFullFieldName(const BaseField *const &obj) {
            return obj->getFullFieldName();
        }

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

    struct DistinctInfo {
        std::vector<const BaseField *> distinctFields;
        std::string distinctOn;
    };

    class BaseQuerySetVirtual : public BaseClass {
    public:
        virtual std::string buildSelect() const = 0;
        virtual std::string getAlias() const = 0;
    };

    template<class T>
    class BaseQuerySet : public BaseQuerySetVirtual {
    public:
        explicit BaseQuerySet(int limit, std::string alias, bool returnInMain = true, const bool join = false) :
            BaseQuerySetVirtual(), _limit(limit), _alias(std::move(alias)), _returnInMain(returnInMain), _join(join) {}

        explicit BaseQuerySet(const std::string_view &alias, bool doAndCheck = false, bool returnInMain = true) :
            BaseQuerySetVirtual(), _alias(alias), _one(true), _doAndCheck(doAndCheck), _returnInMain(returnInMain) {}

        std::vector<WhereClause> filters;
        JoinInfo joinInfo;
        std::vector<std::pair<std::variant<const BaseField *, Function>, bool>> orderFields;
        DistinctInfo distinctInfo;
        std::string _jsonFields;
        std::vector<const BaseField *> onlyFields;
        std::vector<Function> functionsSet;
        std::vector<std::unique_ptr<BaseQuerySetVirtual>> otherQueries;
        std::vector<const BaseField *> groupByFields;
        int _limit{};
        std::string _offset;
        std::string _alias;
        bool _one{};
        bool _doAndCheck{};
        bool _returnInMain{};
        bool _join{};

        [[nodiscard]] std::string getAlias() const override {
            return _alias;
        }

        template<typename U>
        void group_by_impl(U &&u) {
            groupByFields.emplace_back(std::forward<U>(u));
        }

        template<typename U, typename... Args>
        void group_by_impl(U &&u, Args &&...args) {
            groupByFields.emplace_back(std::forward<U>(u));
            group_by_impl(std::forward<Args>(args)...);
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

        [[nodiscard]] std::string buildOnlyFields() const {
            return format("{}",
                          fmt::join(onlyFields | std::views::transform([](const auto &field) {
                                        return field->getFullFieldName();
                                    }),
                                    ", "));
        }

        [[nodiscard]] std::string buildOrderFields() const {
            if(orderFields.empty()) {
                return "";
            }
            return format(" ORDER BY {}",
                          fmt::join(orderFields | std::views::transform([](const auto &pair) {
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

        template<typename U>
        void only_impl(const U &u) {
            onlyFields.emplace_back(u);
        }

        template<typename U, typename... Args>
        void only_impl(const U &u, Args... args) {
            onlyFields.emplace_back(u);
            only_impl(args...);
        }

        template<typename U>
        void distinct_impl(const U &u) {
            distinctInfo.distinctFields.emplace_back(u);
        }

        template<typename U, typename... Args>
        void distinct_impl(const U &u, const Args &...args) {
            distinctInfo.distinctFields.emplace_back(u);
            distinct_impl(args...);
        }

        template<typename U>
        void functions_impl(U &&u) {
            functionsSet.emplace_back(std::forward<U>(u));
        }

        template<typename U, typename... Args>
        void functions_impl(U &&u, Args &&...args) {
            functionsSet.emplace_back(std::forward<U>(u));
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

        template<class U, bool isLeftJoin>
        void join_impl_core(std::string tableReference, const std::string &alias, std::string &&addConditions) {
            std::string lastJoinTable = joinInfo.joinTable.empty() ? T::tableName : joinInfo.joinTable.back().first;

            auto mapFields = U::joinMap();
            auto it = mapFields.find(lastJoinTable);
            if(it == mapFields.end()) {
                it = mapFields.find(T::tableName);
            }
            if(it != mapFields.end()) {
                const auto &[joinFieldFirstTable, joinFieldSecondField] = it->second;
                auto &joinTable = isLeftJoin ? joinInfo.leftJoinTable : joinInfo.joinTable;
                auto &joinCondition = isLeftJoin ? joinInfo.leftJoinCondition : joinInfo.joinCondition;
                auto _aliasJoinedTable = alias.empty() ? joinFieldFirstTable->getTableName() : alias;

                joinTable.emplace_back(std::move(tableReference), alias);
                joinCondition.emplace_back(std::move(fmt::format(R"("{}"."{}" = {} {})",
                                                                 _aliasJoinedTable,
                                                                 joinFieldFirstTable->getFieldName(),
                                                                 joinFieldSecondField->getFullFieldName(),
                                                                 std::move(addConditions))));
            }
        }

        template<class U, bool isLeftJoin>
        void join_impl(const std::string &alias, std::string &&addConditions = "") {
            join_impl_core<U, isLeftJoin>(U::tableName, alias, std::move(addConditions));
        }

        template<class U, bool isLeftJoin>
        void
        join_impl(const BaseQuerySet<U> &querySet, const std::string &alias = "", std::string &&addConditions = "") {
            std::string subquery =
                fmt::format("({}) AS {}", querySet.buildSelect(), alias.empty() ? U::tableName : alias);
            join_impl_core<U, isLeftJoin>(std::move(subquery), alias, std::move(addConditions));
        }

        [[nodiscard]] std::string buildSelectOne() const {
            return fmt::format("{}{} FROM \"{}\" {}{} LIMIT 1{}",
                               _doAndCheck ? "SELECT do_and_check('SELECT " : "SELECT ",
                               _jsonFields.empty()
                                   ? fmt::format("{}{}", buildOnlyFields(), buildFunctions())
                                   : fmt::format("json_build_object({} {})", _jsonFields, buildFunctions()),
                               T::tableName,
                               generateJoinSQL(joinInfo.joinTable, joinInfo.joinCondition, "INNER"),
                               fmt::format("{}{}",
                                           generateJoinSQL(joinInfo.leftJoinTable, joinInfo.leftJoinCondition, "LEFT"),
                                           filter_impl()),
                               _doAndCheck ? "')" : "");
        }

        [[nodiscard]] std::string buildSelect() const override {
            if(_one)
                return buildSelectOne();

            return fmt::format("{} SELECT {} {} {} FROM \"{}\" {} {} {} {} {} {}",
                               buildOtherQueries(),
                               createDistinctClause(),
                               buildOnlyFields(),
                               buildFunctions(),
                               T::tableName,
                               generateJoinSQL(joinInfo.joinTable, joinInfo.joinCondition, "INNER"),
                               generateJoinSQL(joinInfo.leftJoinTable, joinInfo.leftJoinCondition, "LEFT"),
                               filter_impl(),
                               generateGroupBySQL(),
                               buildOrderFields(),
                               limit_impl());
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
            if(this->otherQueries.empty()) {
                return "";
            }

            return format("WITH {}",
                          fmt::join(otherQueries | std::views::transform([](const auto &query) {
                                        return fmt::format("{} AS ({})", query->getAlias(), query->buildSelect());
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

            return !_offset.empty() ? fmt::format(" LIMIT {} OFFSET {}", _limit, _offset)
                                    : fmt::format(" LIMIT {}", _limit);
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
    };

    template<class T>
    class QuerySet final : public BaseQuerySet<T> {

    public:
        explicit QuerySet(const int limit, std::string alias, const bool returnInMain = true, const bool join = false) :
            BaseQuerySet<T>(limit, std::move(alias), returnInMain, join) {}

        explicit QuerySet(const std::string_view &alias,
                          const bool doAndCheck = false,
                          const bool returnInMain = true) : BaseQuerySet<T>(alias, doAndCheck, returnInMain) {}

        [[nodiscard]] std::string alias() const {
            return this->_alias;
        }

        template<typename... Args>
        QuerySet &group_by(Args &&...args) {
            this->group_by_impl(std::forward<Args>(args)...);
            return *this;
        }

        QuerySet &filter(const BaseField *field, const std::string &value, const Operator op = Operator::EQUALS) {
            this->filters.emplace_back(field, value, op);
            return *this;
        }

        QuerySet &
        filter(const BaseField *field, const std::optional<bool> value, const Operator op = Operator::EQUALS) {
            this->filters.emplace_back(field, value, op);
            return *this;
        }

        QuerySet &filter(const BaseField *field1, const BaseField *field2) {
            this->filters.emplace_back(field1, field2);
            return *this;
        }

        QuerySet &filter(WhereClause whereClause) {
            this->filters.push_back(std::move(whereClause));
            return *this;
        }

        QuerySet &order_by(const BaseField *field, bool asc = true) {
            this->orderFields.emplace_back(field, asc);
            return *this;
        }

        template<typename... Args>
        QuerySet &functions(Args &&...args) {
            this->functions_impl(std::forward<Args>(args)...);
            return *this;
        }

        template<class U>
        QuerySet &join(const std::string &alias = "", std::string &&addConditions = "") {
            this->template join_impl<U, false>(alias, std::move(addConditions));
            return *this;
        }

        template<class U>
        QuerySet &join(const QuerySet<U> &other, const std::string &alias = "", std::string addConditions = "") {
            this->template join_impl<U, false>(other, alias, std::move(addConditions));
            return *this;
        }

        template<class U>
        QuerySet &left_join(const std::string &alias = "", std::string &&addConditions = "") {
            this->template join_impl<U, true>(alias, std::move(addConditions));
            return *this;
        }

        template<class U>
        QuerySet &left_join(const QuerySet<U> &other, const std::string &alias = "", std::string addConditions = "") {
            this->template join_impl<U, true>(other, alias, std::move(addConditions));
            return *this;
        }

        template<typename... Args>
        QuerySet &distinct(const Args &...args) {
            this->distinct_impl(args...);
            return *this;
        }

        QuerySet &only(const std::vector<const BaseField *> &fields) {
            for(const auto &field: fields) {
                only(field);
            }
            return *this;
        }

        QuerySet &only(const BaseField *field) {
            this->onlyFields.emplace_back(field);
            return *this;
        }

        template<typename... Args>
        QuerySet &only(const Args &...args) {
            this->only_impl(args...);
            return *this;
        }

        QuerySet &limit(int limit) {
            this->_limit = limit;
            return *this;
        }

        QuerySet &offset(std::string &&offset) {
            this->_offset = std::move(offset);
            return *this;
        }

        QuerySet &jsonFields(std::string &&jsonFields) {
            this->_jsonFields = std::move(jsonFields);
            return *this;
        }

        QuerySet &count(const BaseField *field) {
            functions(Function(fmt::format("count({})::integer", field->getFullFieldName())));
            return *this;
        }

        template<class U>
        QuerySet &addDynamoDbCte(QuerySet<U> cte) {
            this->otherQueries.push_back(std::make_unique<QuerySet<U>>(std::move(cte)));
            return *this;
        }
    };

    class BuildComplexQueries final : public BaseClass {
    public:
        static std::string addQuery_impl() {
            return "";
        }

        template<class U, typename... Args>
        static std::string addQuery_impl(U &&u, Args &&...args) {
            return fmt::format("{} {}", std::forward<U>(u).aliasQuery(), addQuery_impl(std::forward<Args>(args)...));
        }

        static std::string addQueryMain_impl() {
            return "";
        }

        template<class U, typename... Args>
        static std::string addQueryMain_impl(U &&u, Args &&...args) {
            return fmt::format("{} {}",
                               std::forward<U>(u).aliasQueryMain(),
                               addQueryMain_impl(std::forward<Args>(args)...));
        }

        template<typename... Args>
        static std::string buildQuery(Args &&...args) {
            std::string query = removeLastComma(fmt::format("WITH {}", addQuery_impl(std::forward<Args>(args)...)));
            return fmt::format(" {} SELECT json_build_object({}) as result",
                               query,
                               removeLastComma(addQueryMain_impl(std::forward<Args>(args)...)));
        }
    };
}
