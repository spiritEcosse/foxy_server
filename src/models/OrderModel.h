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
#include "decimal.h"

enum class OrderStatus {
    Ordered,  // The item has been ordered by the customer.
    Processing,  // The order is being processed.
    Shipped,  // The item has been shipped to the customer.
    Delivered,  // The item has been delivered to the customer.
    Returned,  // The item has been returned by the customer.
    Refunded,  // The customer has been refunded for the item.
    Cancelled  // The order has been cancelled.
};

namespace api::v1 {
    class OrderModel : public BaseModel<OrderModel> {
    public:
        static inline const std::string tableName = "order";

        struct Field : public BaseModel::Field {
            static inline BaseField status = BaseField("status", tableName);
            static inline BaseField basketId = BaseField("basket_id", tableName);
            static inline BaseField total = BaseField("total", tableName);
            static inline BaseField totalExTaxes = BaseField("total_ex_taxes", tableName);
            static inline BaseField deliveryFees = BaseField("delivery_fees", tableName);
            static inline BaseField taxRate = BaseField("tax_rate", tableName);
            static inline BaseField taxes = BaseField("taxes", tableName);
            static inline BaseField userId = BaseField("user_id", tableName);
            static inline BaseField reference = BaseField("reference", tableName);
            static inline BaseField addressId = BaseField("address_id", tableName);
            static inline BaseField returned = BaseField("returned", tableName);

            Field() : BaseModel::Field() {
                allFields[status.getFieldName()] = status;
                allFields[basketId.getFieldName()] = basketId;
                allFields[total.getFieldName()] = total;
                allFields[totalExTaxes.getFieldName()] = totalExTaxes;
                allFields[deliveryFees.getFieldName()] = deliveryFees;
                allFields[taxRate.getFieldName()] = taxRate;
                allFields[taxes.getFieldName()] = taxes;
                allFields[userId.getFieldName()] = userId;
                allFields[reference.getFieldName()] = reference;
                allFields[addressId.getFieldName()] = addressId;
                allFields[returned.getFieldName()] = returned;
            }
        };

        OrderModel() = default;
        OrderModel(const OrderModel &) = delete;  // Copy constructor
        OrderModel &operator=(const OrderModel &) = delete;  // Copy assignment operator
        OrderModel(OrderModel &&) noexcept = default;  // Move constructor
        OrderModel &operator=(OrderModel &&) noexcept = default;  // Move assignment operator

        std::string status = "ordered";
        int basketId{};
        dec::decimal<2> total{};
        dec::decimal<2> totalExTaxes{};
        dec::decimal<2> deliveryFees{};
        dec::decimal<2> taxRate{};
        dec::decimal<2> taxes{};
        bool returned = false;
        int userId{};
        int addressId{};
        std::string reference;

        explicit OrderModel(const Json::Value &json) : BaseModel(json) {
            basketId = json[Field::basketId.getFieldName()].asInt();
            total = json[Field::total.getFieldName()].asDouble();
            totalExTaxes = json[Field::totalExTaxes.getFieldName()].asDouble();
            deliveryFees = json[Field::deliveryFees.getFieldName()].asDouble();
            taxRate = json[Field::taxRate.getFieldName()].asDouble();
            taxes = json[Field::taxes.getFieldName()].asDouble();
            userId = json[Field::userId.getFieldName()].asInt();
            reference = json[Field::reference.getFieldName()].asString();
            addressId = json[Field::addressId.getFieldName()].asInt();
            returned = json[Field::returned.getFieldName()].asBool();
            auto _status = json[Field::status.getFieldName()].asString();
            if(!_status.empty()) {
                status = _status;
            }

            validateField(Field::status.getFieldName(), status, missingFields);
            validateField(Field::basketId.getFieldName(), basketId, missingFields);
            validateField(Field::total.getFieldName(), total, missingFields);
            validateField(Field::totalExTaxes.getFieldName(), totalExTaxes, missingFields);
            validateField(Field::deliveryFees.getFieldName(), deliveryFees, missingFields);
            validateField(Field::taxRate.getFieldName(), taxRate, missingFields);
            validateField(Field::taxes.getFieldName(), taxes, missingFields);
            validateField(Field::userId.getFieldName(), userId, missingFields);
            validateField(Field::reference.getFieldName(), reference, missingFields);
            validateField(Field::addressId.getFieldName(), addressId, missingFields);
        }

        [[nodiscard]] static std::vector<BaseField> fields();
        [[nodiscard]] std::vector<
            std::pair<BaseField,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
        getObjectValues() const;
        [[nodiscard]] std::string
        sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params) override;
        [[nodiscard]] std::string sqlSelectOne(const std::string &field,
                                               const std::string &value,
                                               const std::map<std::string, std::string, std::less<>> &params) override;
        [[nodiscard]] std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap() const override;
    };
}

#endif  //ORDERMODEL_H
