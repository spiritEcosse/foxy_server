//
// Created by ihor on 18.01.2024.
//

#include "bcrypt.h"
#include "UserModel.h"

using namespace api::v1;

std::vector<std::string> UserModel::fields() {
    return {
        Field::updatedAt,
        Field::email,
        Field::password,
    };
}

std::vector<std::string> UserModel::fullFields() {
    return {
        Field::id,
        Field::email,
        Field::password,
        Field::createdAt,
        Field::updatedAt,
    };
}

std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>> UserModel::getObjectValues() const {
    auto baseValues = BaseModel::getObjectValues();
    baseValues.emplace_back(Field::email, email);
    baseValues.emplace_back(Field::password, password);
    return baseValues;
}

void UserModel::hashPassword() {
    password = bcrypt::generateHash(password);
}

bool UserModel::checkPassword(const std::string& passwordIn) const {
    return bcrypt::validatePassword(passwordIn, this->password);
}

std::string UserModel::sqlAuth(const std::string& email) {
    return "SELECT * FROM \"" + tableName + "\" WHERE email = '" + email + "'";
}
