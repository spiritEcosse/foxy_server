#pragma once

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"
#include "decimal.h"

namespace api::v1 {
    class ShippingProfileModel final : public BaseModel<ShippingProfileModel> {
    public:
        using BaseModel::BaseModel;

        static const inline std::string tableName = "shipping_profile";

        struct Field : BaseModel::Field {
            static inline const auto title = BaseField("title", tableName);
            static inline const auto processingTime = BaseField("processing_time", tableName);
            static inline const auto countryId = BaseField("country_id", tableName);
            static inline const auto postalCode = BaseField("postal_code", tableName);
            static inline const auto shippingUpgradeCost = BaseField("shipping_upgrade_cost", tableName);

            Field() : BaseModel::Field() {
                constexpr std::array fields{&title, &processingTime, &countryId, &postalCode, &shippingUpgradeCost};
                registerFields(fields);
            }
        };

        std::string title;
        int processingTime{};
        int countryId{};  // The country from where the items are shipped.
        std::string postalCode;  // The postal code from where the items are shipped.
        dec::decimal<2> shippingUpgradeCost;  // offer buyers the option to pay more for faster shipping.

        explicit ShippingProfileModel(const Json::Value &json) : BaseModel(json) {
            title = json[Field::title.getFieldName()].asString();
            processingTime = json[Field::processingTime.getFieldName()].asInt();
            countryId = json[Field::countryId.getFieldName()].asInt();
            postalCode = json[Field::postalCode.getFieldName()].asString();
            shippingUpgradeCost = json[Field::shippingUpgradeCost.getFieldName()].asDouble();

            validateField(Field::title.getFieldName(), title, missingFields);
            validateField(Field::processingTime.getFieldName(), processingTime, missingFields);
            validateField(Field::postalCode.getFieldName(), postalCode, missingFields);
            validateField(Field::countryId.getFieldName(), countryId, missingFields);
        }

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
        [[nodiscard]] JoinMap joinMap() const override;
    };
}
