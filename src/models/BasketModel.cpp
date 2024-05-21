//
// Created by ihor on 20.01.2024.
//

#include "BasketModel.h"

using namespace api::v1;

std::vector<std::string> BasketModel::fields() {
    return {};
}

std::vector<std::string> BasketModel::fullFields() {
    return {
        Field::createdAt,
        Field::updatedAt,
    };
}

std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
BasketModel::getObjectValues() const {
    std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        baseValues = {};
    return baseValues;
}
