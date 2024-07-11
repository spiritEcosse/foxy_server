//
// Created by ihor on 08.07.2024.
//

#include "SocialMediaModel.h"
#include "ItemModel.h"

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> SocialMediaModel::joinMap() const {
    return {{ItemModel::tableName, {Field::itemId.getFullFieldName(), ItemModel::Field::id.getFullFieldName()}}};
}

std::vector<BaseField> SocialMediaModel::fields() {
    return {
        Field::title,
        Field::externalId,
        Field::itemId,
    };
}

std::vector<
    std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
SocialMediaModel::getObjectValues() const {
    return {
        {Field::title, title},
        {Field::externalId, externalId},
        {Field::itemId, itemId},
    };
}
