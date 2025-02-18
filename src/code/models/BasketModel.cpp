#include "BasketModel.h"

using namespace api::v1;

BaseModel<BasketModel>::SetMapFieldTypes BasketModel::getObjectValues() const {
    return {{&Field::userId, userId}, {&Field::inUse, inUse}};
}
