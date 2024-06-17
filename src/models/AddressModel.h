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
            static inline BaseField stateAbbr = BaseField("state_abbr", tableName);
            static inline BaseField city = BaseField("city", tableName);
            static inline BaseField zipcode = BaseField("zipcode", tableName);
            static inline BaseField avatar = BaseField("avatar", tableName);
            static inline BaseField userId = BaseField("user_id", tableName);

            Field() : BaseModel::Field() {
                allFields[address.getFieldName()] = address;
                allFields[stateAbbr.getFieldName()] = stateAbbr;
                allFields[city.getFieldName()] = city;
                allFields[zipcode.getFieldName()] = zipcode;
                allFields[avatar.getFieldName()] = avatar;
                allFields[userId.getFieldName()] = userId;
            }
        };

        std::string address;
        std::string stateAbbr;
        std::string city;
        std::string zipcode;
        std::string avatar;
        int userId{};

        AddressModel() = default;

        explicit AddressModel(const Json::Value &json) : BaseModel(json) {
            address = json[Field::address.getFieldName()].asString();
            stateAbbr = json[Field::stateAbbr.getFieldName()].asString();
            city = json[Field::city.getFieldName()].asString();
            zipcode = json[Field::zipcode.getFieldName()].asString();
            avatar = json[Field::avatar.getFieldName()].asString();
            userId = json[Field::userId.getFieldName()].asInt();

            validateField(Field::address.getFieldName(), address, missingFields);
            validateField(Field::stateAbbr.getFieldName(), stateAbbr, missingFields);
            validateField(Field::city.getFieldName(), city, missingFields);
            validateField(Field::zipcode.getFieldName(), zipcode, missingFields);
            validateField(Field::avatar.getFieldName(), avatar, missingFields);
            validateField(Field::userId.getFieldName(), userId, missingFields);
        }

        [[nodiscard]] static std::vector<BaseField> fields();
        [[nodiscard]] std::vector<
            std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
    };
}

#endif  //ADDRESSMODEL_H
