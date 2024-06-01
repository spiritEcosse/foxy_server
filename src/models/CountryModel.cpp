//
// Created by ihor on 21.05.2024.
//

#include "CountryModel.h"

using namespace api::v1;

std::vector<BaseField> CountryModel::fields() {
    return {
        Field::title,
        Field::code,
    };
}

std::vector<std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
CountryModel::getObjectValues() const {
    std::vector<std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        baseValues = {};
    baseValues.emplace_back(Field::title, title);
    baseValues.emplace_back(Field::code, code);
    return baseValues;
}
