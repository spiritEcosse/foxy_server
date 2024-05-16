//
// Created by ihor on 14.01.2024.
//

#ifndef COUNTRYMODEL_H
#define COUNTRYMODEL_H

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"

namespace api::v1 {
    class CountryModel : public BaseModel<CountryModel> {
    public:
        struct Field : public BaseModel::Field {
            static inline const std::string title = "title";
            static inline const std::string code = "code";
        };

        static inline const std::string tableName = "country";

        std::string title;
        std::string code;
        CountryModel() = default;
        CountryModel(const CountryModel&) = delete;  // Copy constructor
        CountryModel& operator=(const CountryModel&) = delete;  // Copy assignment operator
        CountryModel(CountryModel&&) noexcept = default;  // Move constructor
        CountryModel& operator=(CountryModel&&) noexcept = default;  // Move assignment operator

        explicit CountryModel(const Json::Value& json) : BaseModel(json) {
            title = json[Field::title].asString();
            code = json[Field::code].asString();

            Json::Value missingFields;
            validateField(Field::title, title, missingFields);
            validateField(Field::code, code, missingFields);
        }

        [[nodiscard]] static std::vector<std::string> fields();
        [[nodiscard]] static std::vector<std::string> fullFields();
        [[nodiscard]] std::vector<
            std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
    };
}

#endif  //COUNTRYMODEL_H
