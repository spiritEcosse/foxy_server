//
// Created by ihor on 20.01.2024.
//

#include "BasketModel.h"

using namespace api::v1;

std::vector<BaseField<BasketModel>> BasketModel::fields() {
    return {};
}

std::vector<BaseField<BasketModel>> BasketModel::fullFields() {
    return {
        BasketModel::Field::id,
        Field::userId,
        BaseModel::Field::createdAt,
        BaseModel::Field::updatedAt,
    };
}

std::vector<
    std::pair<BaseField<BasketModel>, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
BasketModel::getObjectValues() const {
    std::vector<
        std::pair<BaseField<BasketModel>, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        baseValues = {};
    return baseValues;
}
