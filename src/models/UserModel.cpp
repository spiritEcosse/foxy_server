//
// Created by ihor on 18.01.2024.
//

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

std::vector<std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
UserModel::getObjectValues() const {
    return {
        {Field::email, email},
        {Field::firstName, firstName},
        {Field::lastName, lastName},
        {Field::birthday, birthday},
        {Field::hasNewsletter, hasNewsletter},
        {Field::isAdmin, isAdmin},
    };
}

std::string UserModel::sqlGetOrCreateUser() {
    return fmt::format(
        R"(INSERT INTO "{}" ({}) VALUES {} ON CONFLICT (email) DO UPDATE SET email = EXCLUDED.email, first_name = EXCLUDED.first_name, last_name = EXCLUDED.last_name RETURNING json_build_object({}))",
        tableName,
        fieldsToString(),
        sqlInsertSingle(*this),
        fieldsJsonObject());
}
