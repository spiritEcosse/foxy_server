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

        struct Field {
            static inline const std::string id = "id";
            static inline const std::string slug;
            static inline const std::string createdAt = "created_at";
            static inline const std::string updatedAt = "updated_at";
        };
        static inline const std::string orderBy = Field::updatedAt;
        static inline const std::string primaryKey = Field::id;

        [[nodiscard]] static std::string sqlInsertMultiple(const std::vector<T> &item);
        [[nodiscard]] static std::string sqlInsertSingle(const T &item);
        [[nodiscard]] static std::string sqlInsert(const T &item);
        [[nodiscard]] static std::string sqlUpdate(const T &item);
        [[nodiscard]] static std::string sqlSelectList(int page, int limit);
        [[nodiscard]] static std::string sqlSelectOne(const std::string &field, const std::string &value);
        [[nodiscard]] static std::string fieldsToString();
        [[nodiscard]] static std::string fullFieldsWithTableToString();
        [[nodiscard]] static std::string fieldsJsonObject();
        [[nodiscard]] static std::string sqlDelete(int id);
        void checkMissingFields(const Json::Value& missingFields) const;
        void validateField(const std::string& fieldName, const std::string& value, Json::Value& missingFields) const;
    };
}

#endif  //BASEMODEL_H
