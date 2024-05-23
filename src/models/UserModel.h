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
            static inline BaseField<UserModel> email = BaseField<UserModel>("email");
            static inline BaseField<UserModel> password = BaseField<UserModel>("password");
        };

        std::string email;
        std::string password;
        UserModel() = default;
        UserModel(const UserModel &) = delete;  // Copy constructor
        UserModel &operator=(const UserModel &) = delete;  // Copy assignment operator
        UserModel(UserModel &&) noexcept = default;  // Move constructor
        UserModel &operator=(UserModel &&) noexcept = default;  // Move assignment operator

        explicit UserModel(const Json::Value &json) : BaseModel(json) {
            password = json[Field::password.getFieldName()].asString();
            email = json[Field::email.getFieldName()].asString();

            Json::Value missingFields;
            if(email.empty()) {
                missingFields[Field::email.getFieldName()] = fmt::format("{} is required", Field::email.getFieldName());
            }
            if(password.empty()) {
                missingFields[Field::password.getFieldName()] =
                    fmt::format("{} is required", Field::password.getFieldName());
            }
            hashPassword();
        }

        [[nodiscard]] static std::vector<BaseField<UserModel>> fields();
        [[nodiscard]] static std::vector<BaseField<UserModel>> fullFields();
        [[nodiscard]] std::vector<
            std::pair<BaseField<UserModel>,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
        void hashPassword();
        [[nodiscard]] bool checkPassword(const std::string &passwordIn) const;
        [[nodiscard]] static std::string sqlAuth(const std::string &email);
        [[nodiscard]] static std::string sqlGetOrCreateUser(const std::string &email);
    };
}

#endif  //USERMODEL_H
