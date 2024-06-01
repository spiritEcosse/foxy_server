//
// Created by ihor on 14.01.2024.
//

#include "CountriesIpsModel.h"

using namespace api::v1;

std::vector<BaseField> CountriesIpsModel::fields() {
    return {};
}

std::vector<std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
CountriesIpsModel::getObjectValues() const {
    std::vector<std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        baseValues = {};
    return baseValues;
}
