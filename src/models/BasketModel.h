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
        static inline const std::string tableName = "basket";

        struct Field : public BaseModel::Field {
            static inline BaseField<BasketModel> userId = BaseField<BasketModel>("user_id");
        };

        int userId{};
        BasketModel() = default;
        BasketModel(const BasketModel &) = delete;  // Copy constructor
        BasketModel &operator=(const BasketModel &) = delete;  // Copy assignment operator
        BasketModel(BasketModel &&) noexcept = default;  // Move constructor
        BasketModel &operator=(BasketModel &&) noexcept = default;  // Move assignment operator

        explicit BasketModel(const Json::Value &json) : BaseModel(json) {
            userId = json[Field::userId.getFieldName()].asInt();
            validateField(Field::userId.getFieldName(), userId, missingFields);
        }

        [[nodiscard]] static std::vector<BaseField<BasketModel>> fields();
        [[nodiscard]] static std::vector<BaseField<BasketModel>> fullFields();
        [[nodiscard]] std::vector<
            std::pair<BaseField<BasketModel>,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
    };
}

#endif  //BASKETMODEL_H
