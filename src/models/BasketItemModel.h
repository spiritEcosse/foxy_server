//
// Created by ihor on 14.01.2024.
//

#ifndef BASKETITEMMODEL_H
#define BASKETITEMMODEL_H

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"
#include "OrderModel.h"

namespace api::v1 {
    class BasketItemModel : public BaseModel<BasketItemModel> {
    public:
        struct Field : public BaseModel::Field {
            static inline const std::string basketId = "basket_id";
            static inline const std::string itemId = "item_id";
            static inline const std::string quantity = "quantity";
        };

        static inline const std::string tableName = "basket_item";

        static inline std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap = {
            {OrderModel::tableName, {Field::basketId, OrderModel::Field::basketId}},
        };

        int basketId{};
        int itemId{};
        int quantity{};
        BasketItemModel() = default;
        BasketItemModel(const BasketItemModel&) = delete;  // Copy constructor
        BasketItemModel& operator=(const BasketItemModel&) = delete;  // Copy assignment operator
        BasketItemModel(BasketItemModel&&) noexcept = default;  // Move constructor
        BasketItemModel& operator=(BasketItemModel&&) noexcept = default;  // Move assignment operator

        explicit BasketItemModel(const Json::Value& json) : BaseModel(json) {
            basketId = json[Field::basketId].asInt();
            itemId = json[Field::itemId].asInt();
            quantity = json[Field::quantity].asInt();

            validateField(Field::basketId, basketId, missingFields);
            validateField(Field::itemId, itemId, missingFields);
            validateField(Field::quantity, quantity, missingFields);
        }

        [[nodiscard]] static std::vector<std::string> fields();
        [[nodiscard]] static std::vector<std::string> fullFields();
        [[nodiscard]] std::vector<
            std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
    };
}

#endif  //BASKETITEMMODEL_H
