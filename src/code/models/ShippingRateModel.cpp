#include "ShippingRateModel.h"
#include "ShippingProfileModel.h"
#include "ItemModel.h"

using namespace api::v1;

BaseModelImpl::JoinMap ShippingRateModel::joinMap() {
    return {};
}

BaseModel<ShippingRateModel>::SetMapFieldTypes ShippingRateModel::getObjectValues() const {
    ValueVariant countryIdValue = countryId == 0 ? ValueVariant{std::nullopt} : ValueVariant{countryId};
    return SetMapFieldTypes{{&Field::countryId, countryIdValue},
                            {&Field::shippingProfileId, ValueVariant{shippingProfileId}},
                            {&Field::deliveryDaysMin, ValueVariant{deliveryDaysMin}},
                            {&Field::deliveryDaysMax, ValueVariant{deliveryDaysMax}}};
}

std::string ShippingRateModel::getShippingRateByItem(const BaseField *field, std::string &&value) {
    auto fullCondition =
        WhereClause(&Field::countryId, std::nullopt, Operator::IS) & WhereClause(field, std::move(value));

    QuerySet<ShippingRateModel> qsShipping("shipping", false);
    qsShipping.join<ShippingProfileModel>()
        .join<ItemModel>()
        .filter(std::move(fullCondition))
        .jsonFields(fmt::format("'{0}', {1} + {2}, '{3}', {4} + {2}",
                                Field::deliveryDaysMax.getFieldName(),
                                Field::deliveryDaysMax.getFullFieldName(),
                                ShippingProfileModel::Field::processingTime.getFullFieldName(),
                                Field::deliveryDaysMin.getFieldName(),
                                Field::deliveryDaysMin.getFullFieldName()));
    return BuildComplexQueries::buildQuery(std::move(qsShipping));
}
