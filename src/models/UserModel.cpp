//
// Created by ihor on 18.01.2024.
//

#include "bcrypt.h"
#include "UserModel.h"
#include "OrderModel.h"
#include <fmt/core.h>

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> UserModel::joinMap() const {
    return {
        {OrderModel::tableName,
         {BaseModel<UserModel>::Field::id.getFullFieldName(), OrderModel::Field::userId.getFullFieldName()}},
    };
}

std::vector<BaseField> UserModel::fields() {
    return {
        Field::email,
        Field::password,
        Field::firstName,
        Field::lastName,
        Field::birthday,
        Field::hasNewsletter,
        Field::isAdmin,
    };
}

std::vector<std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
UserModel::getObjectValues() const {
    return {
        {Field::email, email},
        {Field::password, password},
        {Field::firstName, firstName},
        {Field::lastName, lastName},
        {Field::birthday, birthday},
        {Field::hasNewsletter, hasNewsletter},
        {Field::isAdmin, isAdmin},
    };
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

std::string UserModel::sqlGetOrCreateUser() {
    return fmt::format(
        R"(INSERT INTO "{}" ({}) VALUES {} ON CONFLICT (email) DO UPDATE SET email = EXCLUDED.email, first_name = EXCLUDED.first_name, last_name = EXCLUDED.last_name RETURNING json_build_object({}))",
        tableName,
        fieldsToString(),
        sqlInsertSingle(*this),
        fieldsJsonObject());
}
