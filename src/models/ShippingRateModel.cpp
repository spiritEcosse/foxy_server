//
// Created by ihor on 21.05.2024.
//

#include "ShippingRateModel.h"
#include "CountriesIpsModel.h"
#include "ShippingProfileModel.h"
#include "ItemModel.h"

using namespace api::v1;

std::vector<BaseField<ShippingRateModel>> ShippingRateModel::fields() {
    return {
        Field::countryId,
        Field::shippingProfileId,
        Field::deliveryDaysMin,
        Field::deliveryDaysMax,
    };
}

std::vector<BaseField<ShippingRateModel>> ShippingRateModel::fullFields() {
    return {
        BaseModel::Field::id,
        BaseModel::Field::createdAt,
        BaseModel::Field::updatedAt,
        Field::countryId,
        Field::shippingProfileId,
        Field::deliveryDaysMin,
        Field::deliveryDaysMax,
    };
}

std::vector<std::pair<BaseField<ShippingRateModel>,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
ShippingRateModel::getObjectValues() const {
    std::vector<std::pair<BaseField<ShippingRateModel>,
                          std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        baseValues = {};
    if(countryId) {
        baseValues.emplace_back(Field::countryId, countryId);
    } else {
        baseValues.emplace_back(Field::countryId, "Null");
    }
    baseValues.emplace_back(Field::shippingProfileId, shippingProfileId);
    baseValues.emplace_back(Field::deliveryDaysMin, deliveryDaysMin);
    baseValues.emplace_back(Field::deliveryDaysMax, deliveryDaysMax);
    return baseValues;
}

std::string ShippingRateModel::getShippingRateByItem(const std::string &field,
                                                     const std::string &value,
                                                     const std::map<std::string, std::string, std::less<>> &params) {
    std::string app_cloud_name;
    auto it = params.find("client_ip");
    auto clientIp = it->second;

    QuerySet qsCountry(CountriesIpsModel::tableName, "one_country_id", false, false);
    qsCountry
        .filter(CountriesIpsModel::Field::startRange.getFullFieldName(),
                clientIp,
                true,
                std::string("<="),
                std::string("AND"))
        .filter(CountriesIpsModel::Field::endRange.getFullFieldName(), clientIp, true, std::string(">="))
        .only({CountriesIpsModel::Field::countryId.getFullFieldName()});

    QuerySet qsShipping(ShippingRateModel::tableName, "shipping", false);
    qsShipping.join(ShippingProfileModel())
        .join(ItemModel())
        .filter(
            {{ShippingRateModel::Field::countryId.getFullFieldName(),
              "=",
              fmt::format("(SELECT {} FROM {})", CountriesIpsModel::Field::countryId.getFieldName(), qsCountry.alias()),
              false,
              "OR"},
             {ShippingRateModel::Field::countryId.getFullFieldName(), "IS", "NULL", false, ""}})
        .filter(field, std::string(value), true, std::string("="))
        .jsonFields(fmt::format("'{0}', {1} + {2}, '{3}', {4} + {2}",
                                ShippingRateModel::Field::deliveryDaysMax.getFieldName(),
                                ShippingRateModel::Field::deliveryDaysMax.getFullFieldName(),
                                ShippingProfileModel::Field::processingTime.getFullFieldName(),
                                ShippingRateModel::Field::deliveryDaysMin.getFieldName(),
                                ShippingRateModel::Field::deliveryDaysMin.getFullFieldName()));
    return QuerySet::buildQuery(std::move(qsCountry), std::move(qsShipping));
}
