#include "ReviewModel.h"

using namespace api::v1;

BaseModel<ReviewModel>::SetMapFieldTypes ReviewModel::getObjectValues() const {
    return {{&Field::status, status}, {&Field::userId, userId}, {&Field::itemId, itemId}, {&Field::comment, comment}};
}
