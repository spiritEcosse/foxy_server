//
// Created by ihor on 21.05.2024.
//

#include "CountryModel.h"
#include "AddressModel.h"

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> CountryModel::joinMap() const {
    return {
        {AddressModel::tableName,
         {BaseModel<CountryModel>::Field::id.getFullFieldName(), AddressModel::Field::countryId.getFullFieldName()}},
    };
}

std::vector<std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
CountryModel::getObjectValues() const {
    return {
        {Field::title, title},
        {Field::code, code},
    };
}
