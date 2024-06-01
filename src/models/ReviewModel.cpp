//
// Created by ihor on 30.05.2024.
//

#include "ReviewModel.h"

using namespace api::v1;

std::vector<BaseField> ReviewModel::fields() {
    return {
        Field::status,
        Field::userId,
        Field::itemId,
        Field::comment,
    };
}

std::vector<std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
ReviewModel::getObjectValues() const {
    std::vector<std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        baseValues = {};
    baseValues.emplace_back(Field::status, status);
    baseValues.emplace_back(Field::userId, userId);
    baseValues.emplace_back(Field::itemId, itemId);
    baseValues.emplace_back(Field::comment, comment);
    return baseValues;
}
