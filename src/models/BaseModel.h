//
// Created by ihor on 14.01.2024.
//

#ifndef BASEMODEL_H
#define BASEMODEL_H

#include <string>
#include <json/value.h>

namespace api::v1 {
    template<class T>
    class BaseModel {
    public:
        BaseModel() = default;
        BaseModel(const BaseModel &) = delete;  // Copy constructor
        BaseModel &operator=(const BaseModel &) = delete;  // Copy assignment operator
        BaseModel(BaseModel &&) noexcept = default;  // Move constructor
        BaseModel &operator=(BaseModel &&) noexcept = default;  // Move assignment operator
        virtual ~BaseModel() = default;
        std::chrono::system_clock::time_point updatedAt;
        std::chrono::system_clock::time_point createdAt;
        int id = 0;

        explicit BaseModel([[maybe_unused ]] const Json::Value& json) {
            updatedAt = std::chrono::system_clock::now();
        }

        struct Field {
            static inline const std::string id = "id";
            static inline const std::string slug;
            static inline const std::string createdAt = "created_at";
            static inline const std::string updatedAt = "updated_at";
        };
        static inline const std::string orderBy = Field::updatedAt;
        static inline const std::string primaryKey = Field::id;

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
        using ModelFieldKeyHash = decltype(std::unordered_map<std::string, std::string, ModelFieldHasher, std::equal_to<>>());
        static void sqlUpdateSingle(const T &item, ModelFieldKeyHash &uniqueColumns);
        [[nodiscard]] static std::string sqlUpdate(T &&item);
        [[nodiscard]] static std::string sqlSelectList(int page, int limit);
        [[nodiscard]] static std::string sqlSelectOne(const std::string &field, const std::string &value);
        [[nodiscard]] static std::string fieldsToString();
        [[nodiscard]] static std::string fullFieldsWithTableToString();
        [[nodiscard]] static std::string fieldsJsonObject();
        [[nodiscard]] static std::string sqlDelete(int id);
        [[nodiscard]] static std::string sqlDeleteMultiple(const std::vector<int> &ids);
        [[nodiscard]] virtual std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>> getObjectValues() const;
        void checkMissingFields(const Json::Value& missingFields) const;
        void validateField(const std::string& fieldName, const std::string_view& value, Json::Value& missingFields) const;
    };
}

#endif  //BASEMODEL_H
