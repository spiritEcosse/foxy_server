#include "models/UserModel.h"
#include "models/OrderModel.h"
#include <fmt/core.h>

using namespace api::v1;

BaseModelImpl::JoinMap UserModel::joinMap() {
    return {
        {OrderModel::tableName, {&BaseModel::Field::id, &OrderModel::Field::userId}},
    };
}

BaseModel<UserModel>::SetMapFieldTypes UserModel::getObjectValues() const {
    ValueVariant birthdayValue = birthday == "" ? ValueVariant{std::nullopt} : ValueVariant{birthday};
    return {{&Field::email, email},
            {&Field::firstName, firstName},
            {&Field::lastName, lastName},
            {&Field::birthday, birthdayValue},
            {&Field::hasNewsletter, hasNewsletter},
            {&Field::isAdmin, isAdmin}};
}

std::string UserModel::sqlGetOrCreateUser() {
    return fmt::format(
        R"(INSERT INTO "{}" ({}) VALUES {} ON CONFLICT (email) DO UPDATE SET email = EXCLUDED.email, first_name = EXCLUDED.first_name, last_name = EXCLUDED.last_name RETURNING json_build_object({}))",
        tableName,
        fieldsToString(),
        sqlInsertSingle(*this),
        fieldsJsonObject());
}
