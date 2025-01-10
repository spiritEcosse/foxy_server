#pragma once

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"

namespace api::v1 {
    class ShippingRateModel final : public BaseModel<ShippingRateModel> {
    public:
        using BaseModel::BaseModel;

        static const inline std::string tableName = "shipping_rate";

        struct Field : BaseModel::Field {
            static inline const auto countryId = BaseField("country_id", tableName);
            static inline const auto shippingProfileId = BaseField("shipping_profile_id", tableName);
            static inline const auto deliveryDaysMin = BaseField("delivery_days_min", tableName);
            static inline const auto deliveryDaysMax = BaseField("delivery_days_max", tableName);

            Field() : BaseModel::Field() {
                allFields.try_emplace(countryId.getFieldName(), std::cref(countryId));
                allFields.try_emplace(shippingProfileId.getFieldName(), std::cref(shippingProfileId));
                allFields.try_emplace(deliveryDaysMin.getFieldName(), std::cref(deliveryDaysMin));
                allFields.try_emplace(deliveryDaysMax.getFieldName(), std::cref(deliveryDaysMax));
            }
        };

        int countryId{};
        int shippingProfileId{};
        int deliveryDaysMin{};
        int deliveryDaysMax{};

        explicit ShippingRateModel(const Json::Value &json) : BaseModel(json) {
            countryId = json[Field::countryId.getFieldName()].asInt();
            shippingProfileId = json[Field::shippingProfileId.getFieldName()].asInt();
            deliveryDaysMin = json[Field::deliveryDaysMin.getFieldName()].asInt();
            deliveryDaysMax = json[Field::deliveryDaysMax.getFieldName()].asInt();

            validateField(Field::shippingProfileId.getFieldName(), shippingProfileId, missingFields);
            validateField(Field::deliveryDaysMin.getFieldName(), deliveryDaysMin, missingFields);
            validateField(Field::deliveryDaysMax.getFieldName(), deliveryDaysMax, missingFields);
        }

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
        [[nodiscard]] static std::string
        getShippingRateByItem(const std::string &field,
                              const std::string &value,
                              const std::map<std::string, std::string, std::less<>> &params = {});
    };
}
