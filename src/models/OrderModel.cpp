//
// Created by ihor on 20.05.2024.
//

#include "OrderModel.h"
#include "AddressModel.h"
#include "ItemModel.h"
#include "UserModel.h"
#include "BasketModel.h"
#include "BasketItemModel.h"
#include <fmt/core.h>

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> OrderModel::joinMap() const {
    return {{BasketModel::tableName,
             {OrderModel::Field::basketId.getFullFieldName(), BaseModel<BasketModel>::Field::id.getFullFieldName()}},
            {ItemModel::tableName,
             {OrderModel::Field::basketId.getFullFieldName(), BaseModel<ItemModel>::Field::id.getFullFieldName()}},
            {BasketItemModel::tableName,
             {OrderModel::Field::basketId.getFullFieldName(), BasketItemModel::Field::basketId.getFullFieldName()}},
            {AddressModel::tableName,
             {OrderModel::Field::addressId.getFullFieldName(), BaseModel<AddressModel>::Field::id.getFullFieldName()}},
            {UserModel::tableName,
             {OrderModel::Field::userId.getFullFieldName(), BaseModel<UserModel>::Field::id.getFullFieldName()}}};
}

std::vector<
    std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
OrderModel::getObjectValues() const {
    return {
        {Field::status, status},
        {Field::basketId, basketId},
        {Field::total, total},
        {Field::totalExTaxes, totalExTaxes},
        {Field::taxRate, taxRate},
        {Field::taxes, taxes},
        {Field::userId, userId},
        {Field::returned, returned},
        {Field::addressId, addressId},
    };
}

std::string
OrderModel::sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params) {
    QuerySet qsCount = OrderModel().qsCount();
    QuerySet qsPage = OrderModel().qsPage(page, limit);

    QuerySet qs(OrderModel::tableName, limit, "data");
    qs.join(BasketItemModel())
        .only(OrderModel().allSetFields())
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
        .order_by(std::make_pair(BaseModel::Field::updatedAt, false), std::make_pair(BaseModel::Field::id, false))
        .group_by(BaseModel::Field::id,
                  BaseModel::Field::updatedAt,
                  OrderModel::Field::addressId,
                  UserModel::Field::firstName,
                  UserModel::Field::lastName,
                  BaseModel<UserModel>::Field::id,
                  BaseModel<AddressModel>::Field::id,
                  AddressModel::Field::userId,
                  AddressModel::Field::address,
                  AddressModel::Field::city,
                  AddressModel::Field::zipcode,
                  AddressModel::Field::countryId)
        .join(AddressModel())
        .join(UserModel())
        .functions(Function(fmt::format(R"(json_build_object('{}', {}, '{}', {}, '{}', {}) AS user)",
                                        UserModel::Field::firstName.getFieldName(),
                                        UserModel::Field::firstName.getFullFieldName(),
                                        UserModel::Field::lastName.getFieldName(),
                                        UserModel::Field::lastName.getFullFieldName(),
                                        BaseModel<UserModel>::Field::id.getFieldName(),
                                        BaseModel<UserModel>::Field::id.getFullFieldName())),
                   Function(fmt::format(R"(json_build_object({}) AS address)", AddressModel().fieldsJsonObject())),
                   Function(fmt::format(R"(json_agg(json_build_object({})) AS basket_items)",
                                        BasketItemModel().fieldsJsonObject())));

    applyFilters(qs, qsCount, params);
    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}

std::string OrderModel::sqlSelectOne(const std::string &field,
                                     const std::string &value,
                                     [[maybe_unused]] const std::map<std::string, std::string, std::less<>> &params) {
    QuerySet qsBasketItem(ItemModel::tableName, 0, std::string("_items"));
    qsBasketItem.join(BasketItemModel())
        .join(OrderModel())
        .filter(field, value)
        .only(ItemModel::Field::title,
              BaseModel<ItemModel>::Field::id,
              BasketItemModel::Field::quantity,
              BasketItemModel::Field::price);

    QuerySet qsOrder(OrderModel::tableName, "_order", true);
    qsOrder.filter(field, value)
        .jsonFields(addExtraQuotes(OrderModel().fieldsJsonObject()))
        .order_by(std::make_pair(BaseModel<OrderModel>::Field::id, false));
    return QuerySet::buildQuery(std::move(qsOrder), std::move(qsBasketItem));
}
