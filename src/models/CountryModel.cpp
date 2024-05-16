//
// Created by ihor on 21.05.2024.
//

#include "CountryModel.h"

using namespace api::v1;

std::vector<std::string> CountryModel::fields() {
    return {
        Field::title,
        Field::code,
    };
}

std::vector<std::string> CountryModel::fullFields() {
    return {
        Field::id,
        Field::title,
        Field::code,
        Field::createdAt,
        Field::updatedAt,
    };
}

std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
CountryModel::getObjectValues() const {
    auto baseValues = BaseModel::getObjectValues();
    baseValues.emplace_back(Field::title, title);
    baseValues.emplace_back(Field::code, code);
    return baseValues;
}
