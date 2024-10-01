//
// Created by ihor on 28.09.2024.
//

#include "TagModel.h"

#include "ItemModel.h"

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> TagModel::joinMap() const {
    return {
            {ItemModel::tableName,
             {TagModel::Field::itemId.getFullFieldName(), BaseModel<ItemModel>::Field::id.getFullFieldName()}},
        };
}

std::vector<std::pair<BaseField, std::variant<int, bool, std::vector<std::string>, std::string, std::chrono::system_clock::time_point>>>
TagModel::getObjectValues() const {
    return {
        {Field::title, title},
        {Field::itemId, itemId},
        {Field::socialMedia, socialMedia},
    };
}
