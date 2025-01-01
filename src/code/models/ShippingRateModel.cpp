#include "ShippingRateModel.h"
#include "CountriesIpsModel.h"
#include "ShippingProfileModel.h"
#include "ItemModel.h"

using namespace api::v1;

BaseModel<ShippingRateModel>::SetMapFieldTypes ShippingRateModel::getObjectValues() const {
    SetMapFieldTypes baseValues = {};

    if(countryId) {
        baseValues.emplace_back(std::cref(Field::countryId), countryId);
    } else {
        baseValues.emplace_back(std::cref(Field::countryId), "Null");
    }

    baseValues.emplace_back(std::cref(Field::shippingProfileId), shippingProfileId);
    baseValues.emplace_back(std::cref(Field::deliveryDaysMin), deliveryDaysMin);
    baseValues.emplace_back(std::cref(Field::deliveryDaysMax), deliveryDaysMax);

    return baseValues;
}

std::string ShippingRateModel::getShippingRateByItem(const std::string &field,
                                                     const std::string &value,
                                                     const std::map<std::string, std::string, std::less<>> &params) {
    std::string app_cloud_name;
    const auto it = params.find("client_ip");
    const auto clientIp = it->second;

    QuerySet qsCountry(CountriesIpsModel::tableName, "one_country_id", false, false);
    qsCountry
        .filter(CountriesIpsModel::Field::startRange.getFullFieldName(),
                clientIp,
                true,
                std::string("<="),
                std::string("AND"))
        .filter(CountriesIpsModel::Field::endRange.getFullFieldName(), clientIp, true, std::string(">="))
        .only(std::cref(CountriesIpsModel::Field::countryId));

    QuerySet qsShipping(tableName, "shipping", false);
    qsShipping.join(ShippingProfileModel())
        .join(ItemModel())
        .filter(
            {{Field::countryId.getFullFieldName(),
              "=",
              fmt::format("(SELECT {} FROM {})", CountriesIpsModel::Field::countryId.getFieldName(), qsCountry.alias()),
              false,
              "OR"},
             {Field::countryId.getFullFieldName(), "IS", "NULL", false, ""}})
        .filter(field, std::string(value), true, std::string("="))
        .jsonFields(fmt::format("'{0}', {1} + {2}, '{3}', {4} + {2}",
                                Field::deliveryDaysMax.getFieldName(),
                                Field::deliveryDaysMax.getFullFieldName(),
                                ShippingProfileModel::Field::processingTime.getFullFieldName(),
                                Field::deliveryDaysMin.getFieldName(),
                                Field::deliveryDaysMin.getFullFieldName()));
    return QuerySet::buildQuery(std::move(qsCountry), std::move(qsShipping));
}
