//
// Created by ihor on 14.01.2024.
//

#ifndef BASEMODEL_H
#define BASEMODEL_H

#include <string>
#include <json/value.h>
#include <utility>
#include <variant>
#include <chrono>
#include <unordered_map>
#include "QuerySet.h"
#include "decimal.h"

namespace api::v1 {

    template<class T>
    class BaseModel {
    public:
        static inline const std::string tableName;

        struct Field {
            static inline BaseField id = BaseField("id", T::tableName);
            static inline BaseField createdAt = BaseField("created_at", T::tableName);
            static inline BaseField updatedAt = BaseField("updated_at", T::tableName);
            std::map<std::string, BaseField, std::less<>> allFields = {
                {id.getFieldName(), id},
                {createdAt.getFieldName(), createdAt},
                {updatedAt.getFieldName(), updatedAt},
            };
        };

        BaseModel() = default;
        BaseModel(const BaseModel &) = delete;  // Copy constructor
        BaseModel &operator=(const BaseModel &) = delete;  // Copy assignment operator
        BaseModel(BaseModel &&) noexcept = default;  // Move constructor
        BaseModel &operator=(BaseModel &&) noexcept = default;  // Move assignment operator
        virtual ~BaseModel() = default;
        Json::Value missingFields;
        std::chrono::system_clock::time_point updatedAt;
        std::chrono::system_clock::time_point createdAt;
        int id = 0;

        explicit BaseModel([[maybe_unused]] const Json::Value &json) {
            if(json.isMember(Field::id.getFieldName())) {
                id = json[Field::id.getFieldName()].asInt();
            }
        }

        struct ModelFieldHasher {
            std::size_t operator()(std::string_view sv) const {
                std::hash<std::string_view> hasher;
                return hasher(sv);
            }
        };

        [[nodiscard]] virtual std::string sqlInsertMultiple(const std::vector<T> &item);
        [[nodiscard]] virtual std::string sqlInsertSingle(const T &item);
        [[nodiscard]] virtual std::string sqlInsert(const T &item);
        [[nodiscard]] virtual std::string sqlUpdateMultiple(const std::vector<T> &item);
        [[nodiscard]] virtual QuerySet qsCount();
        [[nodiscard]] virtual QuerySet qsPage(int page, int limit);
        using ModelFieldKeyHash =
            decltype(std::unordered_map<std::string, std::string, ModelFieldHasher, std::equal_to<>>());
        virtual void sqlUpdateSingle(const T &item, ModelFieldKeyHash &uniqueColumns);
        [[nodiscard]] virtual std::string sqlUpdate(T &&item);
        [[nodiscard]] virtual std::string
        sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params);
        [[nodiscard]] virtual std::string sqlSelectOne(const std::string &field,
                                                       const std::string &value,
                                                       const std::map<std::string, std::string, std::less<>> &params);
        [[nodiscard]] virtual std::string fieldsToString();
        [[nodiscard]] virtual std::string fieldsJsonObject();
        [[nodiscard]] virtual std::string sqlDelete(int id);
        [[nodiscard]] virtual std::string sqlDeleteMultiple(const std::vector<int> &ids);

        template<class V>
        void validateField(const std::string &fieldName, const V &value, Json::Value &fields) const {
            using VDecayed = std::decay_t<V>;
            // Check if V is int, std::string_view, std::string or dec::decimal<2> and apply appropriate validation
            if constexpr(std::is_same_v<VDecayed, int>) {
                if(!value) {
                    fields[fieldName] = fieldName + " is required";
                }
            } else if constexpr(std::is_same_v<VDecayed, std::string_view> || std::is_same_v<VDecayed, std::string>) {
                if(value.empty()) {
                    fields[fieldName] = fieldName + " is required";
                }
            } else if constexpr(std::is_same_v<VDecayed, dec::decimal<2>>) {
                if(value == dec::decimal<2>(0)) {
                    fields[fieldName] = fieldName + " is required";
                }
            } else if constexpr(std::is_same_v<VDecayed, double>) {
                if(value == 0) {
                    fields[fieldName] = fieldName + " is required";
                }
            }
        }

        [[nodiscard]] bool fieldExists(const std::string &fieldName) const;
        [[nodiscard]] std::vector<BaseField> allSetFields() const;
        [[nodiscard]] virtual std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap() const;
        virtual void applyFilters(QuerySet &qs,
                                  QuerySet &qsCount,
                                  const std::map<std::string, std::string, std::less<>> &params) const;
    };
}

#endif  //BASEMODEL_H
