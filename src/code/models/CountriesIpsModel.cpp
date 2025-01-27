#include "CountriesIpsModel.h"

using namespace api::v1;

BaseModel<CountriesIpsModel>::SetMapFieldTypes CountriesIpsModel::getObjectValues() const {
    return {{&Field::startRange, startRange},
            {&Field::endRange, endRange},
            {&Field::countryCode, countryCode},
            {&Field::countryName, countryName},
            {&Field::countryId, countryId}};
}
