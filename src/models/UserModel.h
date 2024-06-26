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
            static inline BaseField password = BaseField("password", tableName);
            static inline BaseField firstName = BaseField("first_name", tableName);
            static inline BaseField lastName = BaseField("last_name", tableName);
            static inline BaseField birthday = BaseField("birthday", tableName);
            static inline BaseField hasNewsletter = BaseField("has_newsletter", tableName);

            Field() : BaseModel<UserModel>::Field() {
                allFields[email.getFieldName()] = email;
                allFields[password.getFieldName()] = password;
                allFields[firstName.getFieldName()] = firstName;
                allFields[lastName.getFieldName()] = lastName;
                allFields[birthday.getFieldName()] = birthday;
                allFields[hasNewsletter.getFieldName()] = hasNewsletter;
            }
        };

        std::string email;
        std::string password;
        std::string firstName;
        std::string lastName;
        std::string birthday = "Null";
        bool hasNewsletter{};
        UserModel() = default;
        UserModel(const UserModel &) = delete;  // Copy constructor
        UserModel &operator=(const UserModel &) = delete;  // Copy assignment operator
        UserModel(UserModel &&) noexcept = default;  // Move constructor
        UserModel &operator=(UserModel &&) noexcept = default;  // Move assignment operator

        explicit UserModel(const Json::Value &json) : BaseModel(json) {
            password = json[Field::password.getFieldName()].asString();
            email = json[Field::email.getFieldName()].asString();
            firstName = json[Field::firstName.getFieldName()].asString();
            lastName = json[Field::lastName.getFieldName()].asString();
            birthday = json[Field::birthday.getFieldName()].asString();

            validateField(Field::email.getFieldName(), email, missingFields);
            validateField(Field::password.getFieldName(), password, missingFields);
            validateField(Field::firstName.getFieldName(), firstName, missingFields);
            validateField(Field::lastName.getFieldName(), lastName, missingFields);
            validateField(Field::birthday.getFieldName(), birthday, missingFields);
            hashPassword();
        }

        explicit UserModel(const Json::Value &json, bool google) : BaseModel(json) {
            email = json[Field::email.getFieldName()].asString();
            firstName = json["given_name"].asString();
            lastName = json["family_name"].asString();

            validateField(Field::email.getFieldName(), email, missingFields);
            validateField(Field::firstName.getFieldName(), firstName, missingFields);
            validateField(Field::lastName.getFieldName(), lastName, missingFields);
        }

        [[nodiscard]] static std::vector<BaseField> fields();
        [[nodiscard]] std::vector<
            std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
        void hashPassword();
        [[nodiscard]] bool checkPassword(const std::string &passwordIn) const;
        [[nodiscard]] static std::string sqlAuth(const std::string &email);
        [[nodiscard]] std::string sqlGetOrCreateUser();
    };
}

#endif  //USERMODEL_H
