//
// Created by ihor on 18.01.2024.
//

#ifndef USERMODEL_H
#define USERMODEL_H

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"

namespace api::v1 {
    class UserModel : public BaseModel<UserModel> {
    public:
        static inline const std::string tableName = "user";

        struct Field : public BaseModel::Field {
            static inline BaseField email = BaseField("email", tableName);
            static inline BaseField firstName = BaseField("first_name", tableName);
            static inline BaseField lastName = BaseField("last_name", tableName);
            static inline BaseField birthday = BaseField("birthday", tableName);
            static inline BaseField hasNewsletter = BaseField("has_newsletter", tableName);
            static inline BaseField isAdmin = BaseField("is_admin", tableName);

            Field() : BaseModel<UserModel>::Field() {
                allFields[email.getFieldName()] = email;
                allFields[firstName.getFieldName()] = firstName;
                allFields[lastName.getFieldName()] = lastName;
                allFields[birthday.getFieldName()] = birthday;
                allFields[hasNewsletter.getFieldName()] = hasNewsletter;
                allFields[isAdmin.getFieldName()] = hasNewsletter;
            }
        };

        std::string email;
        std::string firstName;
        std::string lastName;
        std::string birthday = "Null";
        bool hasNewsletter{};
        bool isAdmin{};
        UserModel() = default;
        UserModel(const UserModel &) = delete;  // Copy constructor
        UserModel &operator=(const UserModel &) = delete;  // Copy assignment operator
        UserModel(UserModel &&) noexcept = default;  // Move constructor
        UserModel &operator=(UserModel &&) noexcept = default;  // Move assignment operator

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

        explicit UserModel(const Json::Value &json, bool google) : BaseModel(json) {
            email = json[Field::email.getFieldName()].asString();
            firstName = json["given_name"].asString();
            lastName = json["family_name"].asString();

            validateField(Field::email.getFieldName(), email, missingFields);
            validateField(Field::firstName.getFieldName(), firstName, missingFields);
            validateField(Field::lastName.getFieldName(), lastName, missingFields);
        }

        [[nodiscard]] std::vector<
            std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
        [[nodiscard]] std::string sqlGetOrCreateUser();
        [[nodiscard]] std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap() const override;
    };
}

#endif  //USERMODEL_H
