#pragma once
#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"

namespace api::v1 {
    class CountriesIpsModel final : public BaseModel<CountriesIpsModel> {
    public:
        using BaseModel::BaseModel;

        static const inline std::string tableName = "countries_ips";

        struct Field : public BaseModel::Field {
            static inline const auto startRange = BaseField("start_range", tableName);
            static inline const auto endRange = BaseField("end_range", tableName);
            static inline const auto countryCode = BaseField("country_code", tableName);
            static inline const auto countryName = BaseField("country_name", tableName);
            static inline const auto countryId = BaseField("country_id", tableName);

            Field() : BaseModel::Field() {
                constexpr std::array fields{&startRange, &endRange, &countryCode, &countryName, &countryId};
                registerFields(fields);
            }
        };

        std::string startRange;
        std::string endRange;
        std::string countryCode;
        std::string countryName;
        int countryId{};

        explicit CountriesIpsModel(const Json::Value &json) : BaseModel(json) {
            startRange = json[Field::startRange.getFieldName()].asString();
            endRange = json[Field::endRange.getFieldName()].asString();
            countryCode = json[Field::countryCode.getFieldName()].asString();
            countryName = json[Field::countryName.getFieldName()].asString();
            countryId = json[Field::countryId.getFieldName()].asInt();

            validateField(Field::startRange.getFieldName(), startRange, missingFields);
            validateField(Field::endRange.getFieldName(), endRange, missingFields);
            validateField(Field::countryCode.getFieldName(), countryCode, missingFields);
            validateField(Field::countryName.getFieldName(), countryName, missingFields);
            validateField(Field::countryId.getFieldName(), countryId, missingFields);
        }

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
    };
}
