#include "ShippingProfileModel.h"
#include "ShippingRateModel.h"

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> ShippingProfileModel::joinMap() const {
    return {
        {ShippingRateModel::tableName,
         {BaseModel::Field::id.getFullFieldName(), ShippingRateModel::Field::shippingProfileId.getFullFieldName()}}};
}

BaseModel<ShippingProfileModel>::SetMapFieldTypes ShippingProfileModel::getObjectValues() const {
    return {{std::cref(Field::title), title},
            {std::cref(Field::processingTime), processingTime},
            {std::cref(Field::countryId), countryId},
            {std::cref(Field::postalCode), postalCode},
            {std::cref(Field::shippingUpgradeCost), shippingUpgradeCost}};
}
