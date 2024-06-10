//
// Created by ihor on 14.01.2024.
//

#include "BasketItemModel.h"
#include "ItemModel.h"

using namespace api::v1;

template<>
std::map<std::string, std::pair<std::string, std::string>, std::less<>> BaseModel<BasketItemModel>::joinMap = {
    {ItemModel::tableName,
     {BasketItemModel::Field::itemId.getFullFieldName(), BaseModel<ItemModel>::Field::id.getFullFieldName()}},
    {OrderModel::tableName,
     {BasketItemModel::Field::basketId.getFullFieldName(), OrderModel::Field::basketId.getFullFieldName()}}};

std::vector<BaseField> BasketItemModel::fields() {
    return {Field::basketId, Field::itemId, Field::quantity, Field::price};
}

std::vector<
    std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
BasketItemModel::getObjectValues() const {
    return {{Field::basketId, basketId}, {Field::itemId, itemId}, {Field::quantity, quantity}, {Field::price, price}};
};
