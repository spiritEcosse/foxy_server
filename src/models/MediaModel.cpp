//
// Created by ihor on 20.01.2024.
//

#include "MediaModel.h"

using namespace api::v1;

std::vector<std::string> MediaModel::fields() {
    return {
        Field::itemId,
        Field::sort,
        Field::src,
    };
}

std::vector<std::string> MediaModel::fullFields() {
    return {
        Field::id,
        Field::itemId,
        Field::sort,
        Field::src,
        Field::thumb,
        Field::createdAt,
        Field::updatedAt,
    };
}

std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>> MediaModel::getObjectValues() const {
    auto baseValues = BaseModel::getObjectValues();
    baseValues.emplace_back(Field::src, src);
    baseValues.emplace_back(Field::itemId, itemId);
    baseValues.emplace_back(Field::sort, sort);
    return baseValues;
}
