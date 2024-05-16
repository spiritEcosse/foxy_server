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
        struct Field : public BaseModel::Field {
            static inline const std::string startRange = "start_range";
            static inline const std::string endRange = "end_range";
            static inline const std::string countryCode = "country_code";
            static inline const std::string countryName = "country_name";
            static inline const std::string countryId = "country_id";
        };

        static inline const std::string tableName = "countries_ips";

        CountriesIpsModel() = default;
        CountriesIpsModel(const CountriesIpsModel&) = delete;  // Copy constructor
        CountriesIpsModel& operator=(const CountriesIpsModel&) = delete;  // Copy assignment operator
        CountriesIpsModel(CountriesIpsModel&&) noexcept = default;  // Move constructor
        CountriesIpsModel& operator=(CountriesIpsModel&&) noexcept = default;  // Move assignment operator

        using BaseModel::BaseModel;
        [[nodiscard]] static std::vector<std::string> fields();
        [[nodiscard]] static std::vector<std::string> fullFields();
    };
}

#endif  //COUNTRIESIPSMODEL_H
