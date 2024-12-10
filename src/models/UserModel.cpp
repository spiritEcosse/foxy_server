#include "UserModel.h"
#include "OrderModel.h"
#include <fmt/core.h>

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> UserModel::joinMap() const {
    return {
        {OrderModel::tableName,
         {BaseModel::Field::id.getFullFieldName(), OrderModel::Field::userId.getFullFieldName()}},
    };
}

BaseModel<UserModel>::SetMapFieldTypes UserModel::getObjectValues() const {
    return {{std::cref(Field::email), email},
            {std::cref(Field::firstName), firstName},
            {std::cref(Field::lastName), lastName},
            {std::cref(Field::birthday), birthday},
            {std::cref(Field::hasNewsletter), hasNewsletter},
            {std::cref(Field::isAdmin), isAdmin}};
}

std::string UserModel::sqlGetOrCreateUser() {
    return fmt::format(
        R"(INSERT INTO "{}" ({}) VALUES {} ON CONFLICT (email) DO UPDATE SET email = EXCLUDED.email, first_name = EXCLUDED.first_name, last_name = EXCLUDED.last_name RETURNING json_build_object({}))",
        tableName,
        fieldsToString(),
        sqlInsertSingle(*this),
        fieldsJsonObject());
}
