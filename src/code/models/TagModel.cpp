#include "TagModel.h"

#include "ItemModel.h"

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> TagModel::joinMap() const {
    return {
        {ItemModel::tableName, {Field::itemId.getFullFieldName(), BaseModel<ItemModel>::Field::id.getFullFieldName()}},
    };
}

BaseModel<TagModel>::SetMapFieldTypes TagModel::getObjectValues() const {
    return {{std::cref(Field::title), title},
            {std::cref(Field::itemId), itemId},
            {std::cref(Field::socialMedia), socialMedia}};
}
