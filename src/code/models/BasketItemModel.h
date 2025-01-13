#pragma once

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"
#include "OrderModel.h"

namespace api::v1 {
    class BasketItemModel final : public BaseModel<BasketItemModel> {
    public:
        using BaseModel::BaseModel;

        static const inline std::string tableName = "basket_item";

        struct Field : public BaseModel::Field {
            static inline const auto basketId = BaseField("basket_id", tableName);
            static inline const auto itemId = BaseField("item_id", tableName);
            static inline const auto quantity = BaseField("quantity", tableName);
            static inline const auto price = BaseField("price", tableName);

            Field() : BaseModel::Field() {
                allFields.try_emplace(basketId.getFieldName(), std::cref(basketId));
                allFields.try_emplace(itemId.getFieldName(), std::cref(itemId));
                allFields.try_emplace(quantity.getFieldName(), std::cref(quantity));
                allFields.try_emplace(price.getFieldName(), std::cref(price));
            }
        };

        int basketId{};
        int itemId{};
        int quantity = 1;
        dec::decimal<2> price;

        explicit BasketItemModel(const Json::Value &json) : BaseModel(json) {
            if(json.isMember(Field::price.getFieldName())) {
                price = json[Field::price.getFieldName()].asDouble();
            }
            basketId = json[Field::basketId.getFieldName()].asInt();
            itemId = json[Field::itemId.getFieldName()].asInt();

            validateField(Field::basketId.getFieldName(), basketId, missingFields);
            validateField(Field::itemId.getFieldName(), itemId, missingFields);
        }

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
        [[nodiscard]] std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap() const override;
        [[nodiscard]] std::string fieldsJsonObject() override;
        [[nodiscard]] static std::string
        sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params);
    };
}
