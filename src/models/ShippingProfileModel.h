//
// Created by ihor on 14.01.2024.
//

#ifndef SHIPPINGPROFILEMODEL_H
#define SHIPPINGPROFILEMODEL_H

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"
#include "decimal.h"

namespace api::v1 {
    class ShippingProfileModel : public BaseModel<ShippingProfileModel> {
    public:
        static inline const std::string tableName = "shipping_profile";

        struct Field : public BaseModel::Field {
            static inline BaseField title = BaseField("title", tableName);
            static inline BaseField processingTime = BaseField("processing_time", tableName);
            static inline BaseField countryId = BaseField("country_id", tableName);
            static inline BaseField postalCode = BaseField("postal_code", tableName);
            static inline BaseField shippingUpgradeCost = BaseField("shipping_upgrade_cost", tableName);

            Field() : BaseModel<ShippingProfileModel>::Field() {
                allFields[title.getFieldName()] = title;
                allFields[processingTime.getFieldName()] = processingTime;
                allFields[countryId.getFieldName()] = countryId;
                allFields[postalCode.getFieldName()] = postalCode;
                allFields[shippingUpgradeCost.getFieldName()] = shippingUpgradeCost;
            }
        };

        std::string title;
        int processingTime{};
        int countryId{};  // The country from where the items are shipped.
        std::string postalCode;  // The postal code from where the items are shipped.
        dec::decimal<2> shippingUpgradeCost;  // offer buyers the option to pay more for faster shipping.
        ShippingProfileModel() = default;
        ShippingProfileModel(const ShippingProfileModel &) = delete;  // Copy constructor
        ShippingProfileModel &operator=(const ShippingProfileModel &) = delete;  // Copy assignment operator
        ShippingProfileModel(ShippingProfileModel &&) noexcept = default;  // Move constructor
        ShippingProfileModel &operator=(ShippingProfileModel &&) noexcept = default;  // Move assignment operator

        explicit ShippingProfileModel(const Json::Value &json) : BaseModel(json) {
            title = json[Field::title.getFieldName()].asString();
            processingTime = json[Field::processingTime.getFieldName()].asInt();
            countryId = json[Field::countryId.getFieldName()].asInt();
            postalCode = json[Field::postalCode.getFieldName()].asString();
            shippingUpgradeCost = json[Field::shippingUpgradeCost.getFieldName()].asDouble();

            Json::Value missingFields;
            validateField(Field::title.getFieldName(), title, missingFields);
            validateField(Field::processingTime.getFieldName(), processingTime, missingFields);
            validateField(Field::postalCode.getFieldName(), postalCode, missingFields);
            validateField(Field::countryId.getFieldName(), countryId, missingFields);
        }

        [[nodiscard]] static std::vector<BaseField> fields();
        [[nodiscard]] std::vector<
            std::pair<BaseField,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
        getObjectValues() const;
        [[nodiscard]] std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap() const override;
    };
}

#endif  //SHIPPINGPROFILEMODEL_H
