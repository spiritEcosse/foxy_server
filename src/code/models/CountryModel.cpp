#include "models/CountryModel.h"
#include "models/AddressModel.h"

using namespace api::v1;

BaseModelImpl::JoinMap CountryModel::joinMap() {
    return {
        {AddressModel::tableName, {&BaseModel::Field::id, &AddressModel::Field::countryId}},
    };
}

BaseModel<CountryModel>::SetMapFieldTypes CountryModel::getObjectValues() const {
    return {{&Field::title, title}, {&Field::code, code}};
}
