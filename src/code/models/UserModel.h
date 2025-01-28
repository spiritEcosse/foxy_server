#pragma once

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"

namespace api::v1 {
    class UserModel final : public BaseModel<UserModel> {
    public:
        using BaseModel::BaseModel;

        static const inline std::string tableName = "user";

        struct Field : public BaseModel::Field {
            static inline const auto email = BaseField("email", tableName);
            static inline const auto firstName = BaseField("first_name", tableName);
            static inline const auto lastName = BaseField("last_name", tableName);
            static inline const auto birthday = BaseField("birthday", tableName);
            static inline const auto hasNewsletter = BaseField("has_newsletter", tableName);
            static inline const auto isAdmin = BaseField("is_admin", tableName);

            Field() : BaseModel::Field() {
                constexpr std::array fields{&email, &firstName, &lastName, &birthday, &hasNewsletter, &isAdmin};
                registerFields(fields);
            }
        };

        std::string email;
        std::string firstName;
        std::string lastName;
        std::string birthday;
        bool hasNewsletter{};
        bool isAdmin{};

        explicit UserModel(const Json::Value &json) : BaseModel(json) {
            email = json[Field::email.getFieldName()].asString();
            firstName = json[Field::firstName.getFieldName()].asString();
            lastName = json[Field::lastName.getFieldName()].asString();
            birthday = json[Field::birthday.getFieldName()].asString();

            if(json.isMember(Field::isAdmin.getFieldName())) {
                isAdmin = json[Field::isAdmin.getFieldName()].asBool();
            }

            if(json.isMember(Field::hasNewsletter.getFieldName())) {
                hasNewsletter = json[Field::hasNewsletter.getFieldName()].asBool();
            }

            validateField(Field::email.getFieldName(), email, missingFields);
            validateField(Field::firstName.getFieldName(), firstName, missingFields);
            validateField(Field::lastName.getFieldName(), lastName, missingFields);
            validateField(Field::birthday.getFieldName(), birthday, missingFields);
        }

        explicit UserModel(const Json::Value &json, [[maybe_unused]] bool google) : BaseModel(json) {
            email = json[Field::email.getFieldName()].asString();
            firstName = json["given_name"].asString();
            lastName = json["family_name"].asString();

            validateField(Field::email.getFieldName(), email, missingFields);
            validateField(Field::firstName.getFieldName(), firstName, missingFields);
            validateField(Field::lastName.getFieldName(), lastName, missingFields);
        }

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
        [[nodiscard]] std::string sqlGetOrCreateUser();
        [[nodiscard]] JoinMap joinMap() const override;
    };
}
