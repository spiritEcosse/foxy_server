//
// Created by ihor on 14.01.2024.
//

#include "CountriesIpsModel.h"

using namespace api::v1;

std::vector<std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
CountriesIpsModel::getObjectValues() const {
    return {
        {Field::startRange, startRange},
        {Field::endRange, endRange},
        {Field::countryCode, countryCode},
        {Field::countryName, countryName},
        {Field::countryId, countryId},
    };
}
