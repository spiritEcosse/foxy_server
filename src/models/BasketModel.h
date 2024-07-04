//
// Created by ihor on 20.01.2024.
//

#ifndef BASKETMODEL_H
#define BASKETMODEL_H

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"

namespace api::v1 {
    class BasketModel : public BaseModel<BasketModel> {
    public:
        static inline const std::string tableName = "basket";

        struct Field : public BaseModel::Field {
            static inline BaseField userId = BaseField("user_id", tableName);
            static inline BaseField inUse = BaseField("in_use", tableName);

            Field() : BaseModel::Field() {
                allFields[userId.getFieldName()] = userId;
                allFields[inUse.getFieldName()] = inUse;
            }
        };

        int userId{};
        bool inUse{};
        BasketModel() = default;
        BasketModel(const BasketModel &) = delete;  // Copy constructor
        BasketModel &operator=(const BasketModel &) = delete;  // Copy assignment operator
        BasketModel(BasketModel &&) noexcept = default;  // Move constructor
        BasketModel &operator=(BasketModel &&) noexcept = default;  // Move assignment operator

        explicit BasketModel(const Json::Value &json) : BaseModel(json) {
            userId = json[Field::userId.getFieldName()].asInt();
            if(json.isMember(Field::inUse.getFieldName())) {
                inUse = json[Field::inUse.getFieldName()].asBool();
            } else {
                inUse = true;
            }
            validateField(Field::userId.getFieldName(), userId, missingFields);
        }

        [[nodiscard]] static std::vector<BaseField> fields();
        [[nodiscard]] std::vector<
            std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
    };
}

#endif  //BASKETMODEL_H
