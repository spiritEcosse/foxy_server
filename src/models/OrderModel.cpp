//
// Created by ihor on 20.05.2024.
//

#include "OrderModel.h"
#include "ItemModel.h"
#include "BasketModel.h"
#include "BasketItemModel.h"
#include <fmt/core.h>

using namespace api::v1;

template<>
std::map<std::string, std::pair<std::string, std::string>, std::less<>> BaseModel<OrderModel>::joinMap = {
    {BasketModel::tableName,
     {OrderModel::Field::basketId.getFullFieldName(), BaseModel<BasketModel>::Field::id.getFullFieldName()}},
    {ItemModel::tableName,
     {OrderModel::Field::basketId.getFullFieldName(), BaseModel<ItemModel>::Field::id.getFullFieldName()}},
    {BasketItemModel::tableName,
     {OrderModel::Field::basketId.getFullFieldName(), BasketItemModel::Field::basketId.getFullFieldName()}}};

std::vector<BaseField> OrderModel::fields() {
    return {Field::status,
            Field::basketId,
            Field::total,
            Field::totalExTaxes,
            Field::deliveryFees,
            Field::taxRate,
            Field::taxes,
            Field::userId,
            Field::reference,
            Field::addressId};
}

std::vector<
    std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
OrderModel::getObjectValues() const {
    std::vector<std::pair<BaseField,
                          std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
        baseValues = {};
    baseValues.emplace_back(Field::status, status);
    baseValues.emplace_back(Field::basketId, basketId);
    baseValues.emplace_back(Field::total, total);
    baseValues.emplace_back(Field::totalExTaxes, totalExTaxes);
    baseValues.emplace_back(Field::deliveryFees, deliveryFees);
    baseValues.emplace_back(Field::taxRate, taxRate);
    baseValues.emplace_back(Field::taxes, taxes);
    baseValues.emplace_back(Field::userId, userId);
    baseValues.emplace_back(Field::reference, reference);
    baseValues.emplace_back(Field::addressId, addressId);
    return baseValues;
}

std::string
OrderModel::sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params) {
    QuerySet qsCount = OrderModel().qsCount();
    QuerySet qsPage = OrderModel().qsPage(page, limit);

    QuerySet qs(OrderModel::tableName, limit, "data");
    qs.left_join(BasketItemModel())
        .only(OrderModel().allSetFields())
        .functions(Function(fmt::format(R"(COUNT({}) as count_items)", BaseModel::Field::id.getFullFieldName())))
        .order_by(std::make_pair(BaseModel::Field::updatedAt, false), std::make_pair(BaseModel::Field::id, false))
        .group_by(BaseModel::Field::id, BaseModel::Field::updatedAt);

    Field field;
    for(const auto &[key, value]: params) {
        if(fieldExists(key)) {
            qs.filter(field.allFields[key].getFullFieldName(), value);
            qsCount.filter(field.allFields[key].getFullFieldName(), value);
        }
    }
    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}

std::string OrderModel::sqlSelectOne(const std::string &field,
                                     const std::string &value,
                                     [[maybe_unused]] const std::map<std::string, std::string, std::less<>> &params) {
    QuerySet qsBasketItem(ItemModel::tableName, 0, std::string("_items"));
    qsBasketItem.join(BasketItemModel()).join(OrderModel()).filter(field, value).only(ItemModel().allSetFields());

    QuerySet qsOrder(OrderModel::tableName, "_order", true);
    qsOrder.filter(field, value)
        .jsonFields(addExtraQuotes(OrderModel().fieldsJsonObject()))
        .order_by(std::make_pair(BaseModel<OrderModel>::Field::id, false));
    return QuerySet::buildQuery(std::move(qsOrder), std::move(qsBasketItem));
}
