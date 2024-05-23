//
// Created by ihor on 20.05.2024.
//

#ifndef ORDERMODEL_H
#define ORDERMODEL_H

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"
#include "BasketModel.h"

enum class OrderStatus
{
    ordered, delivered, cancelled
};

namespace api::v1
{
class OrderModel: public BaseModel<OrderModel>
{
public:
    static inline const std::string tableName = "order";

    struct Field: public BaseModel::Field
    {
        static inline BaseField<OrderModel> status = BaseField<OrderModel>("status");
        static inline BaseField<OrderModel> basketId = BaseField<OrderModel>("basket_id");
        static inline BaseField<OrderModel> total = BaseField<OrderModel>("total");
        static inline BaseField<OrderModel> totalExTaxes = BaseField<OrderModel>("total_ex_taxes");
        static inline BaseField<OrderModel> deliveryFees = BaseField<OrderModel>("delivery_fees");
        static inline BaseField<OrderModel> taxRate = BaseField<OrderModel>("tax_rate");
        static inline BaseField<OrderModel> taxes = BaseField<OrderModel>("taxes");
        static inline BaseField<OrderModel> userId = BaseField<OrderModel>("user_id");
        static inline BaseField<OrderModel> reference = BaseField<OrderModel>("reference");
    };

    static inline std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap = {
        {BasketModel::tableName, {Field::basketId.getFullFieldName(), getId().getFullFieldName()}},
    };

    OrderModel() = default;
    OrderModel(const OrderModel &) = delete;  // Copy constructor
    OrderModel &operator=(const OrderModel &) = delete;  // Copy assignment operator
    OrderModel(OrderModel &&) noexcept = default;  // Move constructor
    OrderModel &operator=(OrderModel &&) noexcept = default;  // Move assignment operator

    std::string status;
    int basketId{};
    double total{};
    double totalExTaxes{};
    double delivery_fees{};
    double tax_rate{};
    double taxes{};
    int userId{};
    std::string reference;

    explicit OrderModel(const Json::Value &json)
        : BaseModel(json)
    {
        status = json[Field::status.getFieldName()].asString();
        basketId = json[Field::basketId.getFieldName()].asInt();
        total = json[Field::total.getFieldName()].asDouble();
        totalExTaxes = json[Field::totalExTaxes.getFieldName()].asDouble();
        delivery_fees = json[Field::deliveryFees.getFieldName()].asDouble();
        tax_rate = json[Field::taxRate.getFieldName()].asDouble();
        taxes = json[Field::taxes.getFieldName()].asDouble();
        userId = json[Field::userId.getFieldName()].asInt();
        reference = json[Field::reference.getFieldName()].asString();

        validateField(Field::status.getFieldName(), status, missingFields);
        validateField(Field::basketId.getFieldName(), basketId, missingFields);
        validateField(Field::total.getFieldName(), total, missingFields);
        validateField(Field::totalExTaxes.getFieldName(), totalExTaxes, missingFields);
        validateField(Field::deliveryFees.getFieldName(), delivery_fees, missingFields);
        validateField(Field::taxRate.getFieldName(), tax_rate, missingFields);
        validateField(Field::taxes.getFieldName(), taxes, missingFields);
        validateField(Field::userId.getFieldName(), userId, missingFields);
        validateField(Field::reference.getFieldName(), reference, missingFields);
    }

    [[nodiscard]] static std::vector<BaseField<OrderModel>> fields();
    [[nodiscard]] static std::vector<BaseField<OrderModel>> fullFields();
    [[nodiscard]] std::vector<
        std::pair<BaseField<OrderModel>,
                  std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
    getObjectValues() const;
    [[nodiscard]] static std::string sqlSelectList(int page, int limit);
};
}

#endif  //ORDERMODEL_H
