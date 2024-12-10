#include "ReviewModel.h"

using namespace api::v1;

BaseModel<ReviewModel>::SetMapFieldTypes ReviewModel::getObjectValues() const {
    return {{std::cref(Field::status), status},
            {std::cref(Field::userId), userId},
            {std::cref(Field::itemId), itemId},
            {std::cref(Field::comment), comment}};
}
