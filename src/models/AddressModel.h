#ifndef ADDRESSMODEL_H
#define ADDRESSMODEL_H

#include <string>
#include "BaseModel.h"

namespace api::v1 {
    class AddressModel : public BaseModel<AddressModel> {
    public:
        static inline const std::string tableName = "address";

        struct Field : public BaseModel::Field {
            static inline BaseField address = BaseField("address", tableName);
            static inline BaseField city = BaseField("city", tableName);
            static inline BaseField zipcode = BaseField("zipcode", tableName);
            static inline BaseField userId = BaseField("user_id", tableName);
            static inline BaseField countryId = BaseField("country_id", tableName);

            Field() : BaseModel::Field() {
                allFields[address.getFieldName()] = address;
                allFields[city.getFieldName()] = city;
                allFields[zipcode.getFieldName()] = zipcode;
                allFields[userId.getFieldName()] = userId;
                allFields[countryId.getFieldName()] = countryId;
            }
        };

        std::string address;
        std::string city;
        std::string zipcode;
        int countryId;
        int userId{};

        AddressModel() = default;

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

        [[nodiscard]] static std::vector<BaseField> fields();
        [[nodiscard]] std::vector<
            std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
        [[nodiscard]] std::string
        sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params) override;
    };
}

#endif  //ADDRESSMODEL_H
