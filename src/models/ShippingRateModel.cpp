//
// Created by ihor on 21.05.2024.
//

#include "ShippingRateModel.h"
#include "CountriesIpsModel.h"
#include "ShippingProfileModel.h"
#include "ItemModel.h"

using namespace api::v1;

std::vector<std::string> ShippingRateModel::fields() {
    return {
        Field::countryId,
        Field::shippingProfileId,
        Field::deliveryDaysMin,
        Field::deliveryDaysMax,
    };
}

std::vector<std::string> ShippingRateModel::fullFields() {
    return {
        Field::id,
        Field::createdAt,
        Field::updatedAt,
        Field::countryId,
        Field::shippingProfileId,
        Field::deliveryDaysMin,
        Field::deliveryDaysMax,
    };
}

std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
ShippingRateModel::getObjectValues() const {
    auto baseValues = BaseModel::getObjectValues();
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
        .filter(fmt::format("{}.{}", CountriesIpsModel::tableName, CountriesIpsModel::Field::startRange),
                clientIp,
                true,
                std::string("<="),
                std::string("AND"))
        .filter(fmt::format("{}.{}", CountriesIpsModel::tableName, CountriesIpsModel::Field::endRange),
                clientIp,
                true,
                std::string(">="))
        .only({fmt::format("{}.{}", CountriesIpsModel::tableName, CountriesIpsModel::Field::countryId)});

    QuerySet qsShipping(ShippingRateModel::tableName, "shipping", false);
    qsShipping
        .join(ShippingProfileModel::tableName,
              fmt::format("{}.{} = {}.{}",
                          ShippingProfileModel::tableName,
                          ShippingProfileModel::Field::id,
                          ShippingRateModel::tableName,
                          ShippingRateModel::Field::shippingProfileId))
        .join(ItemModel::tableName,
              fmt::format("{}.{} = {}.{}",
                          ShippingProfileModel::tableName,
                          ShippingProfileModel::Field::id,
                          ItemModel::tableName,
                          ItemModel::Field::shippingProfileId))
        .filter({{fmt::format("{}.{}", ShippingRateModel::tableName, ShippingRateModel::Field::countryId),
                  "=",
                  fmt::format("(SELECT {} FROM {})", CountriesIpsModel::Field::countryId, qsCountry.alias()),
                  false,
                  "OR"},
                 {fmt::format("{}.{}", ShippingRateModel::tableName, ShippingRateModel::Field::countryId),
                  "IS",
                  "NULL",
                  false,
                  ""}})
        .filter(fmt::format("{}.{}", ItemModel::tableName, field), std::string(value), true, std::string("="))
        .jsonFields(fmt::format("'{0}', {1}.{0} + {2}.{3}, '{4}', {1}.{4} + {2}.{3}",
                                ShippingRateModel::Field::deliveryDaysMax,
                                ShippingRateModel::tableName,
                                ShippingProfileModel::tableName,
                                ShippingProfileModel::Field::processingTime,
                                ShippingRateModel::Field::deliveryDaysMin));

    return QuerySet::buildQuery(std::move(qsCountry), std::move(qsShipping));
}
