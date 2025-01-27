#pragma once

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"

namespace api::v1 {
    class CountryModel final : public BaseModel<CountryModel> {
    public:
        using BaseModel::BaseModel;
        static const inline std::string tableName = "country";

        struct Field : BaseModel::Field {
            static inline const auto title = BaseField("title", tableName);
            static inline const auto code = BaseField("code", tableName);

            Field() : BaseModel::Field() {
                constexpr std::array fields{&title, &code};
                registerFields(fields);
            }
        };

        std::string title;
        std::string code;

        explicit CountryModel(const Json::Value &json) : BaseModel(json) {
            title = json[Field::title.getFieldName()].asString();
            code = json[Field::code.getFieldName()].asString();

            validateField(Field::title.getFieldName(), title, missingFields);
            validateField(Field::code.getFieldName(), code, missingFields);
        }

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
        [[nodiscard]] JoinMap joinMap() const override;
    };
}
