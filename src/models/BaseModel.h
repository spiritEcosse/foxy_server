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
#include "src/orm/QuerySet.h"

namespace api::v1 {

    template<class T>
    class BaseField {
    public:
        explicit BaseField(std::string fieldName) : fieldName(std::move(fieldName)) {}

        [[nodiscard]] std::string getFieldName() const {
            return fieldName;
        }

        [[nodiscard]] std::string getFullFieldName() const {
            return fmt::format(R"("{}"."{}")", T::tableName, fieldName);
        }

        [[nodiscard]] bool empty() const {
            return fieldName.empty();
        }

    private:
        std::string fieldName;
    };

    template<class T>
    class BaseModel {
    public:
        static inline const std::string tableName;

        struct Field {
            static inline BaseField<T> id = BaseField<T>("id");
            static inline BaseField<T> slug = BaseField<T>("");
            static inline BaseField<T> createdAt = BaseField<T>("created_at");
            static inline BaseField<T> updatedAt = BaseField<T>("updated_at");
        };

        static std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap;

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

        explicit BaseModel([[maybe_unused]] const Json::Value &json) {}

        struct ModelFieldHasher {
            using is_transparent = void;

            std::size_t operator()(std::string_view sv) const {
                std::hash<std::string_view> hasher;
                return hasher(sv);
            }
        };

        [[nodiscard]] static std::string sqlInsertMultiple(const std::vector<T> &item);
        [[nodiscard]] static std::string sqlInsertSingle(const T &item);
        [[nodiscard]] static std::string sqlInsert(const T &item);
        [[nodiscard]] static std::string sqlUpdateMultiple(const std::vector<T> &item);
        [[nodiscard]] static QuerySet qsCount();
        [[nodiscard]] static QuerySet qsPage(int page, int limit);
        using ModelFieldKeyHash =
            decltype(std::unordered_map<std::string, std::string, ModelFieldHasher, std::equal_to<>>());
        static void sqlUpdateSingle(const T &item, ModelFieldKeyHash &uniqueColumns);
        [[nodiscard]] static std::string sqlUpdate(T &&item);
        [[nodiscard]] static std::string sqlSelectList(int page, int limit);
        [[nodiscard]] static std::string
        sqlSelectOne(const std::string &field,
                     const std::string &value,
                     const std::map<std::string, std::string, std::less<>> &params = {});
        [[nodiscard]] static std::string fieldsToString();
        [[nodiscard]] static std::string fullFieldsWithTableToString();
        [[nodiscard]] static std::string fieldsJsonObject();
        [[nodiscard]] static std::string sqlDelete(int id);
        [[nodiscard]] static std::string sqlDeleteMultiple(const std::vector<int> &ids);

        template<class V>
        void validateField(const std::string &fieldName, const V &value, Json::Value &fields) const {
            // Check if V is int or std::string_view and apply appropriate validation
            if constexpr(std::is_same_v<T, int>) {
                if(!value) {
                    fields[fieldName] = fieldName + " is required";
                }
            } else if constexpr(std::is_same_v<T, std::string_view>) {
                if(value.empty()) {
                    fields[fieldName] = fieldName + " is required";
                }
            }
        }

        [[nodiscard]] inline static std::string getPrimaryKeyFieldName() {
            return primaryKey.getFieldName();
        }

        [[nodiscard]] inline static std::string getPrimaryKeyFullName() {
            return primaryKey.getFullFieldName();
        }

        [[nodiscard]] inline static std::string getOrderByFullName() {
            return orderBy.getFullFieldName();
        }

        [[nodiscard]] inline static BaseField<T> getSlug() {
            return Field::slug;
        }

        [[nodiscard]] inline static BaseField<T> getCreatedAt() {
            return Field::createdAt;
        }

        [[nodiscard]] inline static BaseField<T> getUpdatedAt() {
            return Field::updatedAt;
        }

        [[nodiscard]] inline static BaseField<T> getId() {
            return Field::id;
        }

    private:
        static inline BaseField<T> orderBy = Field::updatedAt;
        static inline BaseField<T> primaryKey = Field::id;
    };
}

#endif  //BASEMODEL_H
