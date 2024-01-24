//
// Created by ihor on 18.01.2024.
//

#ifndef USERMODEL_H
#define USERMODEL_H

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"
#include "src/utils/exceptions/RequiredFieldsException.h"

namespace api::v1 {
    class UserModel : public BaseModel<UserModel> {
    public:
        struct Field : public BaseModel::Field {
            static inline const std::string email = "email";
            static inline const std::string password = "password";
        };

        static inline const std::string tableName = "user";
        int id = 0;
        std::string email;
        std::string password;
        std::chrono::system_clock::time_point createdAt;
        std::chrono::system_clock::time_point updatedAt;
        UserModel() = default;
        UserModel(const UserModel&) = delete;  // Copy constructor
        UserModel& operator=(const UserModel&) = delete;  // Copy assignment operator
        UserModel(UserModel&&) noexcept = default;  // Move constructor
        UserModel& operator=(UserModel&&) noexcept = default;  // Move assignment operator

        explicit UserModel(const Json::Value& json) {
            password = json[Field::password].asString();
            email = json[Field::email].asString();

            Json::Value missingFields;
            if(email.empty()) {
                missingFields[Field::email] = Field::email + " is required";
            }
            if(password.empty()) {
                missingFields[Field::password] = Field::password + " is required";
            }
            hashPassword();
            if(!missingFields.empty()) {
                throw RequiredFieldsException(missingFields);
            }
        }

        [[nodiscard]] static std::vector<std::string> fields();
        [[nodiscard]] static std::vector<std::string> fullFields();
        [[nodiscard]] std::vector<std::pair<std::string, std::variant<int, bool, std::string>>> getObjectValues() const;
        void hashPassword();
        [[nodiscard]] bool checkPassword(const std::string& passwordIn) const;
        [[nodiscard]] static std::string sqlAuth(const std::string& email);
    };
}

#endif  //USERMODEL_H
