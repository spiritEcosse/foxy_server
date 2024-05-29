#include "AddressModel.h"

namespace api::v1
{

std::vector<BaseField> AddressModel::fields()
{
    return {
        Field::address,
        Field::stateAbbr,
        Field::city,
        Field::zipcode,
        Field::avatar,
        Field::userId
    };
}

std::vector<std::pair<BaseField,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
AddressModel::getObjectValues() const
{
    return {
        {Field::address, address},
        {Field::stateAbbr, stateAbbr},
        {Field::city, city},
        {Field::zipcode, zipcode},
        {Field::avatar, avatar},
        {Field::userId, userId}
    };
}

}  // namespace api::v1
