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
            static inline BaseField<ShippingProfileModel> title = BaseField<ShippingProfileModel>("title");
            static inline BaseField<ShippingProfileModel> processingTime =
                BaseField<ShippingProfileModel>("processing_time");
            static inline BaseField<ShippingProfileModel> countryId = BaseField<ShippingProfileModel>("country_id");
            static inline BaseField<ShippingProfileModel> postalCode = BaseField<ShippingProfileModel>("postal_code");
            static inline BaseField<ShippingProfileModel> shippingUpgradeCost =
                BaseField<ShippingProfileModel>("shipping_upgrade_cost");
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

        [[nodiscard]] static std::vector<BaseField<ShippingProfileModel>> fields();
        [[nodiscard]] static std::vector<BaseField<ShippingProfileModel>> fullFields();
        [[nodiscard]] std::vector<
            std::pair<BaseField<ShippingProfileModel>,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
        getObjectValues() const;
    };
}

#endif  //SHIPPINGPROFILEMODEL_H
