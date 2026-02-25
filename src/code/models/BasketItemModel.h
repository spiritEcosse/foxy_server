#pragma once

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "models/BaseModel.h"
#include "models/OrderModel.h"

namespace api::v1 {
    class BasketItemModel final : public BaseModel<BasketItemModel> {
    public:
        using BaseModel::BaseModel;

        static const inline std::string tableName = "basket_item";

        struct Field : BaseModel::Field {
            static inline const auto basketId = BaseField("basket_id", tableName);
            static inline const auto itemId = BaseField("item_id", tableName);
            static inline const auto quantity = BaseField("quantity", tableName);
            static inline const auto price = BaseField("price", tableName);

            Field() : BaseModel::Field() {
                constexpr std::array fields{&basketId, &itemId, &quantity, &price};
                registerFields(fields);
            }
        };

        int basketId{};
        int itemId{};
        int quantity = 1;
        std::string price = "0";

        explicit BasketItemModel(const Json::Value &json) : BaseModel(json) {
            if(json.isMember(Field::price.getFieldName())) {
                price = json[Field::price.getFieldName()].asString();
            }
            basketId = json[Field::basketId.getFieldName()].asInt();
            itemId = json[Field::itemId.getFieldName()].asInt();

            validateField(Field::basketId.getFieldName(), basketId, missingFields);
            validateField(Field::itemId.getFieldName(), itemId, missingFields);
        }

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
        [[nodiscard]] std::string fieldsJsonObject() override;
        [[nodiscard]] static std::string
        sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params);

        [[nodiscard]] static JoinMap joinMap();
    };
}
