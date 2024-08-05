//
// Created by ihor on 14.01.2024.
//

#ifndef COUNTRIESIPSMODEL_H
#define COUNTRIESIPSMODEL_H

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"

namespace api::v1 {
    class CountriesIpsModel : public BaseModel<CountriesIpsModel> {
    public:
        static inline const std::string tableName = "countries_ips";

        struct Field : public BaseModel::Field {
            static inline BaseField startRange = BaseField("start_range", tableName);
            static inline BaseField endRange = BaseField("end_range", tableName);
            static inline BaseField countryCode = BaseField("country_code", tableName);
            static inline BaseField countryName = BaseField("country_name", tableName);
            static inline BaseField countryId = BaseField("country_id", tableName);

            Field() : BaseModel<CountriesIpsModel>::Field() {
                allFields[startRange.getFieldName()] = startRange;
                allFields[endRange.getFieldName()] = endRange;
                allFields[countryCode.getFieldName()] = countryCode;
                allFields[countryName.getFieldName()] = countryName;
                allFields[countryId.getFieldName()] = countryId;
            }
        };

        std::string startRange;
        std::string endRange;
        std::string countryCode;
        std::string countryName;
        int countryId{};
        CountriesIpsModel() = default;
        CountriesIpsModel(const CountriesIpsModel &) = delete;  // Copy constructor
        CountriesIpsModel &operator=(const CountriesIpsModel &) = delete;  // Copy assignment operator
        CountriesIpsModel(CountriesIpsModel &&) noexcept = default;  // Move constructor
        CountriesIpsModel &operator=(CountriesIpsModel &&) noexcept = default;  // Move assignment operator

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

        [[nodiscard]] std::vector<
            std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
    };
}

#endif  //COUNTRIESIPSMODEL_H
