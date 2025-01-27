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
            static inline const auto status = BaseField("status", tableName);
            static inline const auto userId = BaseField("user_id", tableName);
            static inline const auto itemId = BaseField("item_id", tableName);
            static inline const auto comment = BaseField("comment", tableName);

            Field() : BaseModel::Field() {
                constexpr std::array fields{&status, &userId, &itemId, &comment};
                registerFields(fields);
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
