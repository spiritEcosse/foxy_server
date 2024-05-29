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

std::vector<BaseField> ShippingProfileModel::fields()
{
    return {
        Field::title,
        Field::processingTime,
        Field::countryId,
        Field::postalCode,
        Field::shippingUpgradeCost,
    };
}

std::vector<std::pair<BaseField,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
ShippingProfileModel::getObjectValues() const
{
    std::vector<std::pair<BaseField,
                          std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
        baseValues = {};
    baseValues.emplace_back(Field::title, title);
    baseValues.emplace_back(Field::processingTime, processingTime);
    baseValues.emplace_back(Field::countryId, countryId);
    baseValues.emplace_back(Field::postalCode, postalCode);
    baseValues.emplace_back(Field::shippingUpgradeCost, shippingUpgradeCost);
    return baseValues;
}
