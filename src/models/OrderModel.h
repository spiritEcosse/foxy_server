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

enum class OrderStatus {
    ordered,
    delivered,
    cancelled
};


namespace api::v1 {
    class OrderModel : public BaseModel<OrderModel> {
    public:
        struct Field : public BaseModel::Field {
            static inline const std::string status = "status";
            static inline const std::string basketId = "basket_id";
            static inline const std::string total = "total";
            static inline const std::string totalExTaxes = "total_ex_taxes";
            static inline const std::string deliveryFees = "delivery_fees";
            static inline const std::string taxRate = "tax_rate";
            static inline const std::string taxes = "taxes";
            static inline const std::string userId = "user_id";
            static inline const std::string reference = "reference";
        };

        static inline const std::string tableName = "order";
        static inline std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap = {
            {BasketModel::tableName, {Field::basketId, BasketModel::Field::id}},
        };

        OrderModel() = default;
        OrderModel(const OrderModel&) = delete;  // Copy constructor
        OrderModel& operator=(const OrderModel&) = delete;  // Copy assignment operator
        OrderModel(OrderModel&&) noexcept = default;  // Move constructor
        OrderModel& operator=(OrderModel&&) noexcept = default;  // Move assignment operator

        std::string status;
        int basketId{};
        double total{};
        double totalExTaxes{};
        double delivery_fees{};
        double tax_rate{};
        double taxes{};
        int userId{};
        std::string reference;

        explicit OrderModel(const Json::Value& json) : BaseModel(json) {
            status = json[Field::status].asString();
            basketId = json[Field::basketId].asInt();
            total = json[Field::total].asDouble();
            totalExTaxes = json[Field::totalExTaxes].asDouble();
            delivery_fees = json[Field::deliveryFees].asDouble();
            tax_rate = json[Field::taxRate].asDouble();
            taxes = json[Field::taxes].asDouble();
            userId = json[Field::userId].asInt();
            reference = json[Field::reference].asString();

            validateField(Field::status, status, missingFields);
            validateField(Field::basketId, basketId, missingFields);
            validateField(Field::total, total, missingFields);
            validateField(Field::totalExTaxes, totalExTaxes, missingFields);
            validateField(Field::deliveryFees, delivery_fees, missingFields);
            validateField(Field::taxRate, tax_rate, missingFields);
            validateField(Field::taxes, taxes, missingFields);
            validateField(Field::userId, userId, missingFields);
            validateField(Field::reference, reference, missingFields);
        }

        [[nodiscard]] static std::vector<std::string> fields();
        [[nodiscard]] static std::vector<std::string> fullFields();
        [[nodiscard]] std::vector<
            std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
        [[nodiscard]] static std::string sqlSelectList(int page, int limit);
    };
}

#endif  //ORDERMODEL_H
