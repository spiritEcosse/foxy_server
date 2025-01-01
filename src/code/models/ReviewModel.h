#pragma once

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"

namespace api::v1 {
    class ReviewModel final : public BaseModel<ReviewModel> {
    public:
        using BaseModel::BaseModel;
        static const inline std::string tableName = "review";

        struct Field : public BaseModel::Field {
            static inline auto status = BaseField("status", tableName);
            static inline auto userId = BaseField("user_id", tableName);
            static inline auto itemId = BaseField("item_id", tableName);
            static inline auto comment = BaseField("comment", tableName);

            Field() : BaseModel::Field() {
                allFields.try_emplace(status.getFieldName(), std::cref(status));
                allFields.try_emplace(userId.getFieldName(), std::cref(userId));
                allFields.try_emplace(itemId.getFieldName(), std::cref(itemId));
                allFields.try_emplace(comment.getFieldName(), std::cref(comment));
            }
        };

        std::string status;
        int userId{};
        int itemId{};
        std::string comment;

        explicit ReviewModel(const Json::Value &json) : BaseModel(json) {
            status = json[Field::status.getFieldName()].asString();
            userId = json[Field::userId.getFieldName()].asInt();
            itemId = json[Field::itemId.getFieldName()].asInt();
            comment = json[Field::comment.getFieldName()].asString();

            validateField(Field::status.getFieldName(), status, missingFields);
            validateField(Field::userId.getFieldName(), userId, missingFields);
            validateField(Field::itemId.getFieldName(), itemId, missingFields);
            validateField(Field::comment.getFieldName(), comment, missingFields);
        }

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
    };
}
