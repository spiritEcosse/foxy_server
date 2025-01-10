#pragma once

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"

namespace api::v1 {
    class BasketModel final : public BaseModel<BasketModel> {
    public:
        using BaseModel::BaseModel;

        static const inline std::string tableName = "basket";

        struct Field : public BaseModel::Field {
            static inline const auto userId = BaseField("user_id", tableName);
            static inline const auto inUse = BaseField("in_use", tableName);

            Field() : BaseModel::Field() {
                allFields.try_emplace(userId.getFieldName(), std::cref(userId));
                allFields.try_emplace(inUse.getFieldName(), std::cref(inUse));
            }
        };

        int userId{};
        bool inUse{};

        explicit BasketModel(const Json::Value &json) : BaseModel(json) {
            userId = json[Field::userId.getFieldName()].asInt();
            if(json.isMember(Field::inUse.getFieldName())) {
                inUse = json[Field::inUse.getFieldName()].asBool();
            } else {
                inUse = true;
            }
            validateField(Field::userId.getFieldName(), userId, missingFields);
        }

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
    };
}
