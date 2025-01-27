#include "ShippingProfileModel.h"
#include "ShippingRateModel.h"

using namespace api::v1;

BaseModelImpl::JoinMap ShippingProfileModel::joinMap() const {
    return {{ShippingRateModel::tableName, {&BaseModel::Field::id, &ShippingRateModel::Field::shippingProfileId}}};
}

BaseModel<ShippingProfileModel>::SetMapFieldTypes ShippingProfileModel::getObjectValues() const {
    return {{&Field::title, title},
            {&Field::processingTime, processingTime},
            {&Field::countryId, countryId},
            {&Field::postalCode, postalCode},
            {&Field::shippingUpgradeCost, shippingUpgradeCost}};
}
