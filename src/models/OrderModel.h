#pragma once

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"
#include "BasketModel.h"
#include "decimal.h"

struct OrderStatus {
    static inline std::string ordered = "Ordered";  // The item has been ordered by the customer.
    static inline std::string processing = "Processing";  // The order is being processed.
    static inline std::string shipped = "Shipped";  // The item has been shipped to the customer.
    static inline std::string delivered = "Delivered";  // The item has been delivered to the customer.
    static inline std::string returned = "Returned";  // The item has been returned by the customer.
    static inline std::string refunded = "Refunded";  // The customer has been refunded for the item.
    static inline std::string cancelled = "Cancelled";  // The order has been cancelled.
};

namespace api::v1 {
    class OrderModel final : public BaseModel<OrderModel> {
    public:
        using BaseModel::BaseModel;

        static const inline std::string tableName = "order";

        struct Field : BaseModel::Field {
            static inline auto status = BaseField("status", tableName);
            static inline auto basketId = BaseField("basket_id", tableName);
            static inline auto total = BaseField("total", tableName);
            static inline auto totalExTaxes = BaseField("total_ex_taxes", tableName);
            static inline auto taxRate = BaseField("tax_rate", tableName);
            static inline auto taxes = BaseField("taxes", tableName);
            static inline auto userId = BaseField("user_id", tableName);
            static inline auto reference = BaseField("reference", tableName);
            static inline auto addressId = BaseField("address_id", tableName);
            static inline auto returned = BaseField("returned", tableName);

            Field() : BaseModel::Field() {
                allFields.try_emplace(status.getFieldName(), std::cref(status));
                allFields.try_emplace(basketId.getFieldName(), std::cref(basketId));
                allFields.try_emplace(total.getFieldName(), std::cref(total));
                allFields.try_emplace(totalExTaxes.getFieldName(), std::cref(totalExTaxes));
                allFields.try_emplace(taxRate.getFieldName(), std::cref(taxRate));
                allFields.try_emplace(taxes.getFieldName(), std::cref(taxes));
                allFields.try_emplace(userId.getFieldName(), std::cref(userId));
                allFields.try_emplace(reference.getFieldName(), std::cref(reference));
                allFields.try_emplace(addressId.getFieldName(), std::cref(addressId));
                allFields.try_emplace(returned.getFieldName(), std::cref(returned));
            }
        };

        std::string status = OrderStatus::ordered;
        int basketId{};
        dec::decimal<2> total{};
        dec::decimal<2> totalExTaxes{};
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
            taxRate = json[Field::taxRate.getFieldName()].asDouble();
            taxes = json[Field::taxes.getFieldName()].asDouble();
            userId = json[Field::userId.getFieldName()].asInt();
            addressId = json[Field::addressId.getFieldName()].asInt();
            returned = json[Field::returned.getFieldName()].asBool();
            if(auto _status = json[Field::status.getFieldName()].asString(); !_status.empty()) {
                status = _status;
            }

            validateField(Field::status.getFieldName(), status, missingFields);
            validateField(Field::basketId.getFieldName(), basketId, missingFields);
            validateField(Field::total.getFieldName(), total, missingFields);
            validateField(Field::totalExTaxes.getFieldName(), totalExTaxes, missingFields);
            validateField(Field::taxRate.getFieldName(), taxRate, missingFields);
            validateField(Field::taxes.getFieldName(), taxes, missingFields);
            validateField(Field::userId.getFieldName(), userId, missingFields);
            validateField(Field::addressId.getFieldName(), addressId, missingFields);
        }

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
        [[nodiscard]] static std::string
        sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params);
        [[nodiscard]] std::string sqlSelectOne(const std::string &field,
                                               const std::string &value,
                                               const std::map<std::string, std::string, std::less<>> &params) override;
        [[nodiscard]] std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap() const override;
    };
}
