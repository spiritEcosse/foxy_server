//
// Created by ihor on 30.05.2024.
//

#include "ReviewModel.h"

using namespace api::v1;

std::vector<std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
ReviewModel::getObjectValues() const {
    return {
        {Field::status, status},
        {Field::userId, userId},
        {Field::itemId, itemId},
        {Field::comment, comment},
    };
}
