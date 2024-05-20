//
// Created by ihor on 20.05.2024.
//

#include "OrderModel.h"

using namespace api::v1;

std::vector<std::string> OrderModel::fields() {
    return {
    };
}

std::vector<std::string> OrderModel::fullFields() {
    return {
        Field::createdAt,
        Field::updatedAt,
    };
}

std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
OrderModel::getObjectValues() const {
    std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>> baseValues = {};
    return baseValues;
}
