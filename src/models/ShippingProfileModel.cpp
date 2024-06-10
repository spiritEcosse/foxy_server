//
// Created by ihor on 21.05.2024.
//

#include "ShippingProfileModel.h"
#include "ShippingRateModel.h"

using namespace api::v1;

template<>
std::map<std::string, std::pair<std::string, std::string>, std::less<>> BaseModel<ShippingProfileModel>::joinMap = {
    {ShippingRateModel::tableName,
     {BaseModel<ShippingProfileModel>::Field::id.getFullFieldName(),
      ShippingRateModel::Field::shippingProfileId.getFullFieldName()}},
};

std::vector<BaseField> ShippingProfileModel::fields() {
    return {
        Field::title,
        Field::processingTime,
        Field::countryId,
        Field::postalCode,
        Field::shippingUpgradeCost,
    };
}

std::vector<
    std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
ShippingProfileModel::getObjectValues() const {
    return {
        {Field::title, title},
        {Field::processingTime, processingTime},
        {Field::countryId, countryId},
        {Field::postalCode, postalCode},
        {Field::shippingUpgradeCost, shippingUpgradeCost},
    };
}
