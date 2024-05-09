//
// Created by ihor on 21.05.2024.
//

#include "ShippingRateModel.h"

using namespace api::v1;

std::vector<std::string> ShippingRateModel::fields() {
    return {
        Field::updatedAt,
        Field::title,
        Field::countryId,
        Field::profileId,
        Field::deliveryDaysMin,
        Field::deliveryDaysMax,
    };
}

std::vector<std::string> ShippingRateModel::fullFields() {
    return {
        Field::id,
        Field::title,
        Field::createdAt,
        Field::updatedAt,
        Field::countryId,
        Field::profileId,
        Field::deliveryDaysMin,
        Field::deliveryDaysMax,
    };
}

std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
ShippingRateModel::getObjectValues() const {
    auto baseValues = BaseModel::getObjectValues();
    baseValues.emplace_back(Field::title, title);
    baseValues.emplace_back(Field::countryId, countryId);
    baseValues.emplace_back(Field::profileId, profileId);
    baseValues.emplace_back(Field::deliveryDaysMin, deliveryDaysMin);
    baseValues.emplace_back(Field::deliveryDaysMax, deliveryDaysMax);
    return baseValues;
}
