//
// Created by ihor on 18.01.2024.
//

#include "bcrypt.h"
#include "UserModel.h"
#include <fmt/core.h>

using namespace api::v1;

std::vector<BaseField<UserModel>> UserModel::fields() {
    return {
        Field::email,
        Field::password,
    };
}

std::vector<BaseField<UserModel>> UserModel::fullFields() {
    return {
        BaseModel::Field::id,
        Field::email,
        Field::password,
        BaseModel::Field::createdAt,
        BaseModel::Field::updatedAt,
    };
}

std::vector<
    std::pair<BaseField<UserModel>, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
UserModel::getObjectValues() const {
    std::vector<
        std::pair<BaseField<UserModel>, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        baseValues = {};
    baseValues.emplace_back(Field::email, email);
    baseValues.emplace_back(Field::password, password);
    return baseValues;
}

void UserModel::hashPassword() {
    password = bcrypt::generateHash(password);
}

bool UserModel::checkPassword(const std::string &passwordIn) const {
    return bcrypt::validatePassword(passwordIn, this->password);
}

std::string UserModel::sqlAuth(const std::string &email) {
    return "SELECT * FROM \"" + tableName + "\" WHERE email = '" + email + "'";
}

std::string UserModel::sqlGetOrCreateUser(const std::string &email) {
    return fmt::format(
        R"(INSERT INTO "{}" (email) VALUES ('{}') ON CONFLICT (email) DO UPDATE SET email = EXCLUDED.email RETURNING json_build_object({}))",
        tableName,
        email,
        fieldsJsonObject());
}
