//
// Created by ihor on 21.05.2024.
//

#include "ShippingProfileModel.h"
#include "ShippingRateModel.h"

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> ShippingProfileModel::joinMap() const {
    return {{ShippingRateModel::tableName,
             {BaseModel<ShippingProfileModel>::Field::id.getFullFieldName(),
              ShippingRateModel::Field::shippingProfileId.getFullFieldName()}}};
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
