#include "models/TagModel.h"

#include "models/ItemModel.h"

using namespace api::v1;

BaseModelImpl::JoinMap TagModel::joinMap() {
    return {
        {ItemModel::tableName, {&Field::itemId, &BaseModel<ItemModel>::Field::id}},
    };
}

BaseModel<TagModel>::SetMapFieldTypes TagModel::getObjectValues() const {
    return {{&Field::title, title}, {&Field::itemId, itemId}, {&Field::socialMedia, socialMedia}};
}
