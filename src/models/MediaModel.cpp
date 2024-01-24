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

std::vector<std::pair<std::string, std::variant<int, bool, std::string>>> MediaModel::getObjectValues() const {
    return {
        {Field::itemId, itemId},
        {Field::sort, sort},
        {Field::src, src},
    };
}
