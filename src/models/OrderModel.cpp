//
// Created by ihor on 20.05.2024.
//

#include "OrderModel.h"
#include "BasketModel.h"
#include "BasketItemModel.h"
#include <fmt/core.h>

using namespace api::v1;

std::vector<BaseField> OrderModel::fields() {
    return {
        Field::status,
        Field::basketId,
        Field::total,
        Field::totalExTaxes,
        Field::deliveryFees,
        Field::taxRate,
        Field::taxes,
        Field::userId,
        Field::reference,
        Field::addressId
    };
}

std::vector<
    std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
OrderModel::getObjectValues() const {
    std::vector<
        std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
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

std::string OrderModel::sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params) {
    QuerySet qsCount = OrderModel().qsCount();
    QuerySet qsPage = OrderModel().qsPage(page, limit);

    QuerySet qs(OrderModel::tableName, limit, "data");
    qs.left_join(BasketItemModel())
        .only({OrderModel::fullFieldsWithTableToString(),
               fmt::format(R"(COUNT({}) as count_items)", BaseModel::Field::id.getFullFieldName())})
        .order_by(std::make_pair(BaseModel::Field::updatedAt.getFullFieldName(), false),
                  std::make_pair(BaseModel::Field::id.getFullFieldName(), false))
        .group_by(BaseModel::Field::id.getFullFieldName(), BaseModel::Field::updatedAt.getFullFieldName());

    Field field;
    for (const auto &[key, value]: params) {
        if (fieldExists(key)) {
            qs.filter(field.allFields[key].getFullFieldName(), value);
            qsCount.filter(field.allFields[key].getFullFieldName(), value);
        }
    }
    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}
