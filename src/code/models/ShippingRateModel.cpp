#include "ShippingRateModel.h"
#include "CountriesIpsModel.h"
#include "ShippingProfileModel.h"
#include "ItemModel.h"

using namespace api::v1;

BaseModel<ShippingRateModel>::SetMapFieldTypes ShippingRateModel::getObjectValues() const {
    return {{&Field::countryId, countryId ? countryId : std::nullopt},
            {&Field::shippingProfileId, shippingProfileId},
            {&Field::deliveryDaysMin, deliveryDaysMin},
            {&Field::deliveryDaysMax, deliveryDaysMax}};
}

std::string ShippingRateModel::getShippingRateByItem(const BaseField *field,
                                                     const std::string &value,
                                                     const std::map<std::string, std::string, std::less<>> &params) {
    std::string app_cloud_name;
    const auto it = params.find("client_ip");
    const auto clientIp = it->second;

    QuerySet qsCountry(CountriesIpsModel::tableName, "one_country_id", false, false);
    qsCountry.filter(&CountriesIpsModel::Field::startRange, clientIp, Operator::LESS_OR_EQUAL)
        .filter(&CountriesIpsModel::Field::endRange, clientIp, Operator::GREATER_OR_EQUAL)
        .only(&CountriesIpsModel::Field::countryId);

    const auto countryIdIsNull = WhereClause(&Field::countryId, std::nullopt, Operator::IS);
    const auto countryIdIsValue = WhereClause(
        &Field::countryId,
        fmt::format("(SELECT {} FROM {})", CountriesIpsModel::Field::countryId.getFieldName(), qsCountry.alias()));
    QuerySet qsShipping(tableName, "shipping", false);
    qsShipping.join(ShippingProfileModel())
        .join(ItemModel())
        .filter(countryIdIsNull | countryIdIsValue)
        .filter(field, value)
        .jsonFields(fmt::format("'{0}', {1} + {2}, '{3}', {4} + {2}",
                                Field::deliveryDaysMax.getFieldName(),
                                Field::deliveryDaysMax.getFullFieldName(),
                                ShippingProfileModel::Field::processingTime.getFullFieldName(),
                                Field::deliveryDaysMin.getFieldName(),
                                Field::deliveryDaysMin.getFullFieldName()));
    return QuerySet::buildQuery(std::move(qsCountry), std::move(qsShipping));
}
