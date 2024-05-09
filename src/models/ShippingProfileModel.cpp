//
// Created by ihor on 21.05.2024.
//

#include "ShippingProfileModel.h"

using namespace api::v1;

std::vector<std::string> ShippingProfileModel::fields() {
    return {
        Field::title,
        Field::processingTime,
        Field::countryId,
        Field::postalCode,
        Field::shippingUpgradeCost,
    };
}

std::vector<std::string> ShippingProfileModel::fullFields() {
    return {
        Field::id,
        Field::title,
        Field::createdAt,
        Field::updatedAt,
        Field::processingTime,
        Field::countryId,
        Field::postalCode,
        Field::shippingUpgradeCost,
    };
}

std::vector<std::pair<std::string,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
ShippingProfileModel::getObjectValues() const {
    std::vector<std::pair<std::string,
                          std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
        baseValues = {};
    baseValues.emplace_back(Field::title, title);
    baseValues.emplace_back(Field::processingTime, processingTime);
    baseValues.emplace_back(Field::countryId, countryId);
    baseValues.emplace_back(Field::postalCode, postalCode);
    baseValues.emplace_back(Field::shippingUpgradeCost, shippingUpgradeCost);
    return baseValues;
}
