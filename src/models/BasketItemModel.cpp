//
// Created by ihor on 14.01.2024.
//

#include "BasketItemModel.h"

using namespace api::v1;

std::vector<std::string> BasketItemModel::fields() {
    return {};
}

std::vector<std::string> BasketItemModel::fullFields() {
    return {
        Field::createdAt,
        Field::updatedAt,
    };
}

std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
BasketItemModel::getObjectValues() const {
    std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        baseValues = {};
    return baseValues;
}
