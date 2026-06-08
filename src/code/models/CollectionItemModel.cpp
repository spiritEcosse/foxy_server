#include "models/CollectionItemModel.h"
#include "models/ItemModel.h"
#include "models/CollectionModel.h"

using namespace api::v1;

BaseModelImpl::JoinMap CollectionItemModel::joinMap() {
    return {{ItemModel::tableName, {&Field::itemId, &BaseModel<ItemModel>::Field::id}},
            {CollectionModel::tableName, {&Field::collectionId, &BaseModel<CollectionModel>::Field::id}}};
}

BaseModel<CollectionItemModel>::SetMapFieldTypes CollectionItemModel::getObjectValues() const {
    return {{&Field::collectionId, collectionId}, {&Field::itemId, itemId}};
}
