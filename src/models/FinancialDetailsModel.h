//
// Created by ihor on 07.07.2024.
//

#ifndef FINANCIALDETAILSMODEL_H
#define FINANCIALDETAILSMODEL_H

#include <decimal.h>
#include "BaseModel.h"

namespace api::v1 {
    class FinancialDetailsModel : public BaseModel<FinancialDetailsModel> {
    public:
        static inline const std::string tableName = "financial_details";

        struct Field : public BaseModel::Field {
            static inline BaseField taxRate = BaseField("tax_rate", tableName);

            Field() : BaseModel::Field() {
                allFields[taxRate.getFieldName()] = taxRate;
            }
        };

        FinancialDetailsModel() = default;
        FinancialDetailsModel(const FinancialDetailsModel &) = delete;  // Copy constructor
        FinancialDetailsModel &operator=(const FinancialDetailsModel &) = delete;  // Copy assignment operator
        FinancialDetailsModel(FinancialDetailsModel &&) noexcept = default;  // Move constructor
        FinancialDetailsModel &operator=(FinancialDetailsModel &&) noexcept = default;  // Move assignment operator

        explicit FinancialDetailsModel(const Json::Value &json) : BaseModel(json) {
            taxRate = json[Field::taxRate.getFieldName()].asDouble();

            validateField(Field::taxRate.getFieldName(), taxRate, missingFields);
        }

        dec::decimal<2> taxRate;
        [[nodiscard]] static std::vector<BaseField> fields();
        [[nodiscard]] std::vector<
            std::pair<BaseField,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
        getObjectValues() const;
    };
}
#endif  //FINANCIALDETAILSMODEL_H
