#pragma once

#include <string>
#include "BaseModel.h"

namespace api::v1 {
    class AddressModel final : public BaseModel<AddressModel> {
    public:
        using BaseModel::BaseModel;

        static const inline std::string tableName = "address";

        struct Field : BaseModel::Field {
            static inline const auto address = BaseField("address", tableName);
            static inline const auto city = BaseField("city", tableName);
            static inline const auto zipcode = BaseField("zipcode", tableName);
            static inline const auto userId = BaseField("user_id", tableName);
            static inline const auto countryId = BaseField("country_id", tableName);

            Field() : BaseModel::Field() {
                allFields.try_emplace(address.getFieldName(), std::cref(address));
                allFields.try_emplace(city.getFieldName(), std::cref(city));
                allFields.try_emplace(zipcode.getFieldName(), std::cref(zipcode));
                allFields.try_emplace(userId.getFieldName(), std::cref(userId));
                allFields.try_emplace(countryId.getFieldName(), std::cref(countryId));
            }
        };

        std::string address;
        std::string city;
        std::string zipcode;
        int countryId{};
        int userId{};

        explicit AddressModel(const Json::Value &json) : BaseModel(json) {
            address = json[Field::address.getFieldName()].asString();
            city = json[Field::city.getFieldName()].asString();
            zipcode = json[Field::zipcode.getFieldName()].asString();
            userId = json[Field::userId.getFieldName()].asInt();
            countryId = json[Field::countryId.getFieldName()].asInt();

            validateField(Field::address.getFieldName(), address, missingFields);
            validateField(Field::city.getFieldName(), city, missingFields);
            validateField(Field::zipcode.getFieldName(), zipcode, missingFields);
            validateField(Field::userId.getFieldName(), userId, missingFields);
            validateField(Field::countryId.getFieldName(), countryId, missingFields);
        }

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
        [[nodiscard]] static std::string
        sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params);
        [[nodiscard]] std::string fieldsJsonObject() override;
        [[nodiscard]] std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap() const override;
    };
}
