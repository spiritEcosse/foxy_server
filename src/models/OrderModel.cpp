//
// Created by ihor on 20.05.2024.
//

#include "OrderModel.h"
#include "BasketModel.h"
#include "BasketItemModel.h"
#include <fmt/core.h>

using namespace api::v1;

std::vector<std::string> OrderModel::fields() {
    return {};
}

std::vector<std::string> OrderModel::fullFields() {
    return {
        Field::id,
        Field::status,
        Field::basketId,
        Field::total,
        Field::totalExTaxes,
        Field::deliveryFees,
        Field::taxRate,
        Field::taxes,
        Field::userId,
        Field::reference,
        Field::createdAt,
        Field::updatedAt,
    };
}

std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
OrderModel::getObjectValues() const {
    std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        baseValues = {};
    return baseValues;
}

std::string OrderModel::sqlSelectList(int page, int limit) {
    QuerySet qsCount = OrderModel::qsCount();
    QuerySet qsPage = OrderModel::qsPage(page, limit);

    QuerySet qs(OrderModel::tableName, limit, "data");
    qs.join(BasketItemModel())
        .only({OrderModel::fullFieldsWithTableToString(),
               fmt::format(R"(COUNT("{}".{}) as count_items)", BasketItemModel::tableName, BasketItemModel::Field::id)})
        .order_by(std::make_pair(fmt::format(R"("{}".{})", OrderModel::tableName, OrderModel::Field::updatedAt), false),
                  std::make_pair(fmt::format(R"("{}".{})", OrderModel::tableName, OrderModel::Field::id), false))
        .group_by(fmt::format(R"("{}".{})", OrderModel::tableName, OrderModel::Field::id),
                  fmt::format(R"("{}".{})", OrderModel::tableName, OrderModel::Field::updatedAt));

    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}
