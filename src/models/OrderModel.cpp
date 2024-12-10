#include "OrderModel.h"
#include "AddressModel.h"
#include "ItemModel.h"
#include "UserModel.h"
#include "BasketModel.h"
#include "BasketItemModel.h"
#include <fmt/core.h>

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> OrderModel::joinMap() const {
    return {
        {BasketModel::tableName,
         {Field::basketId.getFullFieldName(), BaseModel<BasketModel>::Field::id.getFullFieldName()}},
        {ItemModel::tableName,
         {Field::basketId.getFullFieldName(), BaseModel<ItemModel>::Field::id.getFullFieldName()}},
        {BasketItemModel::tableName,
         {Field::basketId.getFullFieldName(), BasketItemModel::Field::basketId.getFullFieldName()}},
        {AddressModel::tableName,
         {Field::addressId.getFullFieldName(), BaseModel<AddressModel>::Field::id.getFullFieldName()}},
        {UserModel::tableName, {Field::userId.getFullFieldName(), BaseModel<UserModel>::Field::id.getFullFieldName()}}};
}

BaseModel<OrderModel>::SetMapFieldTypes OrderModel::getObjectValues() const {
    return {{std::cref(Field::status), status},
            {std::cref(Field::basketId), basketId},
            {std::cref(Field::total), total},
            {std::cref(Field::totalExTaxes), totalExTaxes},
            {std::cref(Field::taxRate), taxRate},
            {std::cref(Field::taxes), taxes},
            {std::cref(Field::userId), userId},
            {std::cref(Field::returned), returned},
            {std::cref(Field::addressId), addressId}};
}

std::string
OrderModel::sqlSelectList(const int page, int limit, const std::map<std::string, std::string, std::less<>> &params) {
    QuerySet qsCount = OrderModel::qsCount();
    QuerySet qsPage = OrderModel::qsPage(page, limit);

    QuerySet qs(tableName, limit, "data");
    qs.join(BasketItemModel())
        .only(allSetFields())
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
        .order_by(std::make_pair(std::cref(BaseModel::Field::updatedAt), false),
                  std::make_pair(std::cref(BaseModel::Field::id), false))
        .group_by(std::cref(BaseModel::Field::id),
                  std::cref(BaseModel::Field::updatedAt),
                  std::cref(Field::addressId),
                  std::cref(UserModel::Field::firstName),
                  std::cref(UserModel::Field::lastName),
                  std::cref(BaseModel<UserModel>::Field::id),
                  std::cref(BaseModel<AddressModel>::Field::id),
                  std::cref(AddressModel::Field::userId),
                  std::cref(AddressModel::Field::address),
                  std::cref(AddressModel::Field::city),
                  std::cref(AddressModel::Field::zipcode),
                  std::cref(AddressModel::Field::countryId))
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
        .only(std::cref(ItemModel::Field::title),
              std::cref(BaseModel<ItemModel>::Field::id),
              std::cref(BasketItemModel::Field::quantity),
              std::cref(BasketItemModel::Field::price));

    QuerySet qsOrder(tableName, "_order", true);
    qsOrder.filter(field, value)
        .jsonFields(addExtraQuotes(OrderModel().fieldsJsonObject()))
        .order_by(std::make_pair(std::cref(BaseModel::Field::id), false));
    return QuerySet::buildQuery(std::move(qsOrder), std::move(qsBasketItem));
}
