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
        static inline const std::string tableName = "country";

        struct Field : public BaseModel::Field {
            static inline BaseField title = BaseField("title", tableName);
            static inline BaseField code = BaseField("code", tableName);

            Field() : BaseModel<CountryModel>::Field() {
                allFields[title.getFieldName()] = title;
                allFields[code.getFieldName()] = code;
            }
        };

        std::string title;
        std::string code;
        CountryModel() = default;
        CountryModel(const CountryModel &) = delete;  // Copy constructor
        CountryModel &operator=(const CountryModel &) = delete;  // Copy assignment operator
        CountryModel(CountryModel &&) noexcept = default;  // Move constructor
        CountryModel &operator=(CountryModel &&) noexcept = default;  // Move assignment operator

        explicit CountryModel(const Json::Value &json) : BaseModel(json) {
            title = json[Field::title.getFieldName()].asString();
            code = json[Field::code.getFieldName()].asString();

            validateField(Field::title.getFieldName(), title, missingFields);
            validateField(Field::code.getFieldName(), code, missingFields);
        }

        [[nodiscard]] std::vector<
            std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
        [[nodiscard]] std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap() const override;
    };
}

#endif  //COUNTRYMODEL_H
