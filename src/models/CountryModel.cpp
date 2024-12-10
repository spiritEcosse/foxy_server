#include "CountryModel.h"
#include "AddressModel.h"

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> CountryModel::joinMap() const {
    return {
        {AddressModel::tableName,
         {BaseModel::Field::id.getFullFieldName(), AddressModel::Field::countryId.getFullFieldName()}},
    };
}

BaseModel<CountryModel>::SetMapFieldTypes CountryModel::getObjectValues() const {
    return {{std::cref(Field::title), title}, {std::cref(Field::code), code}};
}
