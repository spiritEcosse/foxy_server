#include "BasketModel.h"

using namespace api::v1;

BaseModel<BasketModel>::SetMapFieldTypes BasketModel::getObjectValues() const {
    return {{std::cref(Field::userId), userId}, {std::cref(Field::inUse), inUse}};
}
