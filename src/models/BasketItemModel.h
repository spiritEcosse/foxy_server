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
        static inline const std::string tableName = "basket_item";

        struct Field : public BaseModel::Field {
            static inline BaseField<BasketItemModel> basketId = BaseField<BasketItemModel>("basket_id");
            static inline BaseField<BasketItemModel> itemId = BaseField<BasketItemModel>("item_id");
            static inline BaseField<BasketItemModel> quantity = BaseField<BasketItemModel>("quantity");
        };

        static inline std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap = {
            {OrderModel::tableName,
             {Field::basketId.getFullFieldName(), OrderModel::Field::basketId.getFullFieldName()}},
        };

        int basketId{};
        int itemId{};
        int quantity{};
        BasketItemModel() = default;
        BasketItemModel(const BasketItemModel &) = delete;  // Copy constructor
        BasketItemModel &operator=(const BasketItemModel &) = delete;  // Copy assignment operator
        BasketItemModel(BasketItemModel &&) noexcept = default;  // Move constructor
        BasketItemModel &operator=(BasketItemModel &&) noexcept = default;  // Move assignment operator

        explicit BasketItemModel(const Json::Value &json) : BaseModel(json) {
            basketId = json[Field::basketId.getFieldName()].asInt();
            itemId = json[Field::itemId.getFieldName()].asInt();
            quantity = json[Field::quantity.getFieldName()].asInt();

            validateField(Field::basketId.getFieldName(), basketId, missingFields);
            validateField(Field::itemId.getFieldName(), itemId, missingFields);
            validateField(Field::quantity.getFieldName(), quantity, missingFields);
        }

        [[nodiscard]] static std::vector<BaseField<BasketItemModel>> fields();
        [[nodiscard]] static std::vector<BaseField<BasketItemModel>> fullFields();
        [[nodiscard]] std::vector<
            std::pair<BaseField<BasketItemModel>,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
    };
}

#endif  //BASKETITEMMODEL_H
