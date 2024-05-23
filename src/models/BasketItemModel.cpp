//
// Created by ihor on 14.01.2024.
//

#include "BasketItemModel.h"

using namespace api::v1;

std::vector<BaseField<BasketItemModel>> BasketItemModel::fields() {
    return {};
}

std::vector<BaseField<BasketItemModel>> BasketItemModel::fullFields() {
    return {
        BaseModel::Field::id,
        Field::basketId,
        Field::itemId,
        Field::quantity,
        BaseModel::Field::createdAt,
        BaseModel::Field::updatedAt,
    };
}

std::vector<
    std::pair<BaseField<BasketItemModel>, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
BasketItemModel::getObjectValues() const {
    std::vector<std::pair<BaseField<BasketItemModel>,
                          std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        baseValues = {};
    return baseValues;
}
