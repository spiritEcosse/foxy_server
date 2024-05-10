//
// Created by ihor on 21.05.2024.
//

#include "ShippingRateModel.h"

using namespace api::v1;

std::vector<std::string> ShippingRateModel::fields() {
    return {
        Field::countryId,
        Field::shippingProfileId,
        Field::deliveryDaysMin,
        Field::deliveryDaysMax,
    };
}

std::vector<std::string> ShippingRateModel::fullFields() {
    return {
        Field::id,
        Field::createdAt,
        Field::updatedAt,
        Field::countryId,
        Field::shippingProfileId,
        Field::deliveryDaysMin,
        Field::deliveryDaysMax,
    };
}

std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
ShippingRateModel::getObjectValues() const {
    auto baseValues = BaseModel::getObjectValues();
    if (countryId) {
        baseValues.emplace_back(Field::countryId, countryId);
    } else {
        baseValues.emplace_back(Field::countryId, "Null");
    }
    baseValues.emplace_back(Field::shippingProfileId, shippingProfileId);
    baseValues.emplace_back(Field::deliveryDaysMin, deliveryDaysMin);
    baseValues.emplace_back(Field::deliveryDaysMax, deliveryDaysMax);
    return baseValues;
}
