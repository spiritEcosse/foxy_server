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
        static inline const std::string tableName = "shipping_rate";

        struct Field : public BaseModel::Field {
            static inline BaseField countryId = BaseField("country_id", tableName);
            static inline BaseField shippingProfileId = BaseField("shipping_profile_id", tableName);
            static inline BaseField deliveryDaysMin = BaseField("delivery_days_min", tableName);
            static inline BaseField deliveryDaysMax = BaseField("delivery_days_max", tableName);

            Field() : BaseModel<ShippingRateModel>::Field() {
                allFields[countryId.getFieldName()] = countryId;
                allFields[shippingProfileId.getFieldName()] = shippingProfileId;
                allFields[deliveryDaysMin.getFieldName()] = deliveryDaysMin;
                allFields[deliveryDaysMax.getFieldName()] = deliveryDaysMax;
            }
        };

        int countryId{};
        int shippingProfileId{};
        int deliveryDaysMin{};
        int deliveryDaysMax{};
        ShippingRateModel() = default;
        ShippingRateModel(const ShippingRateModel &) = delete;  // Copy constructor
        ShippingRateModel &operator=(const ShippingRateModel &) = delete;  // Copy assignment operator
        ShippingRateModel(ShippingRateModel &&) noexcept = default;  // Move constructor
        ShippingRateModel &operator=(ShippingRateModel &&) noexcept = default;  // Move assignment operator

        explicit ShippingRateModel(const Json::Value &json) : BaseModel(json) {
            countryId = json[Field::countryId.getFieldName()].asInt();
            shippingProfileId = json[Field::shippingProfileId.getFieldName()].asInt();
            deliveryDaysMin = json[Field::deliveryDaysMin.getFieldName()].asInt();
            deliveryDaysMax = json[Field::deliveryDaysMax.getFieldName()].asInt();

            Json::Value missingFields;
            validateField(Field::shippingProfileId.getFieldName(), shippingProfileId, missingFields);
            validateField(Field::deliveryDaysMin.getFieldName(), deliveryDaysMin, missingFields);
            validateField(Field::deliveryDaysMax.getFieldName(), deliveryDaysMax, missingFields);
        }

        [[nodiscard]] static std::vector<BaseField> fields();
        [[nodiscard]] std::vector<
            std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
        [[nodiscard]] static std::string
        getShippingRateByItem(const std::string &field,
                              const std::string &value,
                              const std::map<std::string, std::string, std::less<>> &params = {});
    };
}

#endif  //SHIPPINGPRATEMODEL_H
