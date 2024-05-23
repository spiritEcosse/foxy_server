//
// Created by ihor on 20.05.2024.
//

#include "OrderModel.h"
#include "BasketModel.h"
#include "BasketItemModel.h"
#include <fmt/core.h>

using namespace api::v1;

std::vector<BaseField<OrderModel>> OrderModel::fields() {
    return {};
}

std::vector<BaseField<OrderModel>> OrderModel::fullFields() {
    return {
        BaseModel::Field::id,
        Field::status,
        Field::basketId,
        Field::total,
        Field::totalExTaxes,
        Field::deliveryFees,
        Field::taxRate,
        Field::taxes,
        Field::userId,
        Field::reference,
        BaseModel::Field::createdAt,
        BaseModel::Field::updatedAt,
    };
}

std::vector<
    std::pair<BaseField<OrderModel>, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
OrderModel::getObjectValues() const {
    std::vector<
        std::pair<BaseField<OrderModel>, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        baseValues = {};
    return baseValues;
}

std::string OrderModel::sqlSelectList(int page, int limit) {
    QuerySet qsCount = OrderModel::qsCount();
    QuerySet qsPage = OrderModel::qsPage(page, limit);

    QuerySet qs(OrderModel::tableName, limit, "data");
    qs.join(BasketItemModel())
        .only({OrderModel::fullFieldsWithTableToString(),
               fmt::format(R"(COUNT({}) as count_items)", BaseModel::Field::id.getFullFieldName())})
        .order_by(std::make_pair(BaseModel::Field::updatedAt.getFullFieldName(), false),
                  std::make_pair(BaseModel::Field::id.getFullFieldName(), false))
        .group_by(BaseModel::Field::id.getFullFieldName(), BaseModel::Field::updatedAt.getFullFieldName());

    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}
