#include "models/ShippingProfileModel.h"
#include "models/ShippingRateModel.h"

using namespace api::v1;

BaseModelImpl::JoinMap ShippingProfileModel::joinMap() {
    return {{ShippingRateModel::tableName, {&BaseModel::Field::id, &ShippingRateModel::Field::shippingProfileId}}};
}

BaseModel<ShippingProfileModel>::SetMapFieldTypes ShippingProfileModel::getObjectValues() const {
    return {{&Field::title, title},
            {&Field::processingTime, processingTime},
            {&Field::countryId, countryId},
            {&Field::postalCode, postalCode},
            {&Field::shippingUpgradeCost, shippingUpgradeCost.empty() ? ValueVariant{std::nullopt} : ValueVariant{shippingUpgradeCost}}};
}
