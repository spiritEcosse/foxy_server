#include "TagModel.h"

#include "ItemModel.h"

using namespace api::v1;

BaseModelImpl::JoinMap TagModel::joinMap() const {
    return {
        {ItemModel::tableName, {&Field::itemId, &BaseModel<ItemModel>::Field::id}},
    };
}

BaseModel<TagModel>::SetMapFieldTypes TagModel::getObjectValues() const {
    return {{&Field::title, title}, {&Field::itemId, itemId}, {&Field::socialMedia, socialMedia}};
}
