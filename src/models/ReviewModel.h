//
// Created by ihor on 14.01.2024.
//

#ifndef REVIEWMODEL_H
#define REVIEWMODEL_H

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"

namespace api::v1 {
    class ReviewModel : public BaseModel<ReviewModel> {
    public:
        static inline const std::string tableName = "review";

        struct Field : public BaseModel::Field {
            static inline BaseField status = BaseField("status", tableName);
            static inline BaseField userId = BaseField("user_id", tableName);
            static inline BaseField itemId = BaseField("item_id", tableName);
            static inline BaseField comment = BaseField("comment", tableName);

            Field() : BaseModel<ReviewModel>::Field() {
                allFields[status.getFieldName()] = status;
                allFields[userId.getFieldName()] = userId;
                allFields[itemId.getFieldName()] = itemId;
                allFields[comment.getFieldName()] = comment;
            }
        };

        std::string status;
        int userId{};
        int itemId{};
        std::string comment;

        ReviewModel() = default;
        ReviewModel(const ReviewModel &) = delete;  // Copy constructor
        ReviewModel &operator=(const ReviewModel &) = delete;  // Copy assignment operator
        ReviewModel(ReviewModel &&) noexcept = default;  // Move constructor
        ReviewModel &operator=(ReviewModel &&) noexcept = default;  // Move assignment operator

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

        [[nodiscard]] static std::vector<BaseField> fields();
        [[nodiscard]] std::vector<
            std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
    };
}

#endif  //REVIEWMODEL_H
