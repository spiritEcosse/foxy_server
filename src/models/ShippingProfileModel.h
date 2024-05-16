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
        struct Field : public BaseModel::Field {
            static inline const std::string title = "title";
            static inline const std::string processingTime = "processing_time";
            static inline const std::string countryId = "country_id";
            static inline const std::string postalCode = "postal_code";
            static inline const std::string shippingUpgradeCost = "shipping_upgrade_cost";
        };

        static inline const std::string tableName = "shipping_profile";

        std::string title;
        int processingTime{};
        int countryId{};  // The country from where the items are shipped.
        std::string postalCode;  // The postal code from where the items are shipped.
        dec::decimal<2> shippingUpgradeCost;  // offer buyers the option to pay more for faster shipping.
        ShippingProfileModel() = default;
        ShippingProfileModel(const ShippingProfileModel&) = delete;  // Copy constructor
        ShippingProfileModel& operator=(const ShippingProfileModel&) = delete;  // Copy assignment operator
        ShippingProfileModel(ShippingProfileModel&&) noexcept = default;  // Move constructor
        ShippingProfileModel& operator=(ShippingProfileModel&&) noexcept = default;  // Move assignment operator

        explicit ShippingProfileModel(const Json::Value& json) : BaseModel(json) {
            title = json[Field::title].asString();
            processingTime = json[Field::processingTime].asInt();
            countryId = json[Field::countryId].asInt();
            postalCode = json[Field::postalCode].asString();
            shippingUpgradeCost = json[Field::shippingUpgradeCost].asDouble();

            Json::Value missingFields;
            validateField(Field::title, title, missingFields);
            validateField(Field::processingTime, processingTime, missingFields);
            validateField(Field::postalCode, postalCode, missingFields);
            validateField(Field::countryId, countryId, missingFields);
        }

        [[nodiscard]] static std::vector<std::string> fields();
        [[nodiscard]] static std::vector<std::string> fullFields();
        [[nodiscard]] std::vector<
            std::pair<std::string,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
        getObjectValues() const;
    };
}

#endif  //SHIPPINGPROFILEMODEL_H
