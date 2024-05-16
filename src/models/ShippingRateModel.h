//
// Created by ihor on 14.01.2024.
//

#ifndef SHIPPINGPRATEMODEL_H
#define SHIPPINGPRATEMODEL_H

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"

namespace api::v1 {
    class ShippingRateModel : public BaseModel<ShippingRateModel> {
    public:
        struct Field : public BaseModel::Field {
            static inline const std::string countryId = "country_id";
            static inline const std::string shippingProfileId = "shipping_profile_id";
            static inline const std::string deliveryDaysMin = "delivery_days_min";
            static inline const std::string deliveryDaysMax = "delivery_days_max";
        };

        static inline const std::string tableName = "shipping_rate";

        int countryId{};
        int shippingProfileId{};
        int deliveryDaysMin{};
        int deliveryDaysMax{};
        ShippingRateModel() = default;
        ShippingRateModel(const ShippingRateModel&) = delete;  // Copy constructor
        ShippingRateModel& operator=(const ShippingRateModel&) = delete;  // Copy assignment operator
        ShippingRateModel(ShippingRateModel&&) noexcept = default;  // Move constructor
        ShippingRateModel& operator=(ShippingRateModel&&) noexcept = default;  // Move assignment operator

        explicit ShippingRateModel(const Json::Value& json) : BaseModel(json) {
            countryId = json[Field::countryId].asInt();
            shippingProfileId = json[Field::shippingProfileId].asInt();
            deliveryDaysMin = json[Field::deliveryDaysMin].asInt();
            deliveryDaysMax = json[Field::deliveryDaysMax].asInt();

            Json::Value missingFields;
            validateField(Field::shippingProfileId, shippingProfileId, missingFields);
            validateField(Field::deliveryDaysMin, deliveryDaysMin, missingFields);
            validateField(Field::deliveryDaysMax, deliveryDaysMax, missingFields);
        }

        [[nodiscard]] static std::vector<std::string> fields();
        [[nodiscard]] static std::vector<std::string> fullFields();
        [[nodiscard]] std::vector<
            std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
        [[nodiscard]] static std::string
        getShippingRateByItem(const std::string& field,
                              const std::string& value,
                              const std::map<std::string, std::string, std::less<>>& params = {});
    };
}

#endif  //SHIPPINGPRATEMODEL_H
