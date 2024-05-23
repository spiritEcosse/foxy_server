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
            static inline BaseField<CountriesIpsModel> startRange = BaseField<CountriesIpsModel>("start_range");
            static inline BaseField<CountriesIpsModel> endRange = BaseField<CountriesIpsModel>("end_range");
            static inline BaseField<CountriesIpsModel> countryCode = BaseField<CountriesIpsModel>("country_code");
            static inline BaseField<CountriesIpsModel> countryName = BaseField<CountriesIpsModel>("country_name");
            static inline BaseField<CountriesIpsModel> countryId = BaseField<CountriesIpsModel>("country_id");
        };

        CountriesIpsModel() = default;
        CountriesIpsModel(const CountriesIpsModel &) = delete;  // Copy constructor
        CountriesIpsModel &operator=(const CountriesIpsModel &) = delete;  // Copy assignment operator
        CountriesIpsModel(CountriesIpsModel &&) noexcept = default;  // Move constructor
        CountriesIpsModel &operator=(CountriesIpsModel &&) noexcept = default;  // Move assignment operator

        using BaseModel::BaseModel;
        [[nodiscard]] static std::vector<BaseField<CountriesIpsModel>> fields();
        [[nodiscard]] static std::vector<BaseField<CountriesIpsModel>> fullFields();
        [[nodiscard]] std::vector<
            std::pair<BaseField<CountriesIpsModel>,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
    };
}

#endif  //COUNTRIESIPSMODEL_H
