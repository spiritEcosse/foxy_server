//
// Created by ihor on 20.01.2024.
//

#include "BasketModel.h"

using namespace api::v1;

std::vector<BaseField> BasketModel::fields()
{
    return {};
}

std::vector<
    std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
BasketModel::getObjectValues() const
{
    std::vector<
        std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        baseValues = {};
    return baseValues;
}
