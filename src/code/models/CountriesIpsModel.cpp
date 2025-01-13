#include "CountriesIpsModel.h"

using namespace api::v1;

BaseModel<CountriesIpsModel>::SetMapFieldTypes CountriesIpsModel::getObjectValues() const {
    return {{std::cref(Field::startRange), startRange},
            {std::cref(Field::endRange), endRange},
            {std::cref(Field::countryCode), countryCode},
            {std::cref(Field::countryName), countryName},
            {std::cref(Field::countryId), countryId}};
}
