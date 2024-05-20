//
// Created by ihor on 20.01.2024.
//

#ifndef BASKETMODEL_H
#define BASKETMODEL_H

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"

namespace api::v1 {
    class BasketModel : public BaseModel<BasketModel> {
    public:
        struct Field : public BaseModel::Field {
            static inline const std::string userId = "user_id";
        };

        static inline const std::string tableName = "basket";

        int userId{};
        BasketModel() = default;
        BasketModel(const BasketModel&) = delete;  // Copy constructor
        BasketModel& operator=(const BasketModel&) = delete;  // Copy assignment operator
        BasketModel(BasketModel&&) noexcept = default;  // Move constructor
        BasketModel& operator=(BasketModel&&) noexcept = default;  // Move assignment operator

        explicit BasketModel(const Json::Value& json) : BaseModel(json) {
            userId = json[Field::userId].asInt();
            validateField(Field::userId, userId, missingFields);
        }

        [[nodiscard]] static std::vector<std::string> fields();
        [[nodiscard]] static std::vector<std::string> fullFields();
        [[nodiscard]] std::vector<
            std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
    };
}

#endif  //BASKETMODEL_H
