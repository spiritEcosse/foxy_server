#include "OrderModel.h"
#include "AddressModel.h"
#include "ItemModel.h"
#include "UserModel.h"
#include "BasketModel.h"
#include "BasketItemModel.h"
#include <fmt/core.h>

using namespace api::v1;

BaseModelImpl::JoinMap OrderModel::joinMap() const {
    return {{BasketModel::tableName, {&Field::basketId, &BaseModel<BasketModel>::Field::id}},
            {ItemModel::tableName, {&Field::basketId, &BaseModel<ItemModel>::Field::id}},
            {BasketItemModel::tableName, {&Field::basketId, &BasketItemModel::Field::basketId}},
            {AddressModel::tableName, {&Field::addressId, &BaseModel<AddressModel>::Field::id}},
            {UserModel::tableName, {&Field::userId, &BaseModel<UserModel>::Field::id}}};
}

BaseModel<OrderModel>::SetMapFieldTypes OrderModel::getObjectValues() const {
    return {{&Field::status, status},
            {&Field::basketId, basketId},
            {&Field::total, total},
            {&Field::totalExTaxes, totalExTaxes},
            {&Field::taxRate, taxRate},
            {&Field::taxes, taxes},
            {&Field::userId, userId},
            {&Field::returned, returned},
            {&Field::addressId, addressId}};
}

std::string
OrderModel::sqlSelectList(const int page, int limit, const std::map<std::string, std::string, std::less<>> &params) {
    QuerySet qsCount = OrderModel::qsCount();
    QuerySet qsPage = OrderModel::qsPage(page, limit);

    QuerySet qs(tableName, limit, "data");
    qs.join(BasketItemModel())
        .only(allSetFields())
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
        .order_by(std::make_pair(&BaseModel::Field::updatedAt, false), std::make_pair(&BaseModel::Field::id, false))
        .group_by(&BaseModel::Field::id,
                  &BaseModel::Field::updatedAt,
                  &Field::addressId,
                  &UserModel::Field::firstName,
                  &UserModel::Field::lastName,
                  &BaseModel<UserModel>::Field::id,
                  &BaseModel<AddressModel>::Field::id,
                  &AddressModel::Field::userId,
                  &AddressModel::Field::address,
                  &AddressModel::Field::city,
                  &AddressModel::Field::zipcode,
                  &AddressModel::Field::countryId)
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

std::string OrderModel::sqlSelectOne(const BaseField *field,
                                     const std::string &value,
                                     [[maybe_unused]] const std::map<std::string, std::string, std::less<>> &params) {
    QuerySet qsBasketItem(ItemModel::tableName, 0, ItemModel::tableName, false);
    qsBasketItem.join(BasketItemModel())
        .filter(&BasketItemModel::Field::itemId, &BaseModel::Field::id)
        .functions(Function(fmt::format("json_agg(json_build_object({}))", ItemModel().fieldsJsonObject())));

    QuerySet qsOrder(tableName, tableName, true, true);
    qsOrder.filter(field, value)
        .jsonFields(addExtraQuotes(OrderModel().fieldsJsonObject()))
        .functions(Function(fmt::format(R"( 'items', COALESCE(({}), '[]'::json))", qsBasketItem.buildSelect())));

    return qsOrder.buildSelectOne();
}
