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
            static inline BaseField gateway = BaseField("gateway", tableName);
            static inline BaseField gatewayMerchantId = BaseField("gateway_merchant_id", tableName);
            static inline BaseField merchantId = BaseField("merchant_id", tableName);
            static inline BaseField merchantName = BaseField("merchant_name", tableName);

            Field() : BaseModel::Field() {
                allFields[taxRate.getFieldName()] = taxRate;
                allFields[gateway.getFieldName()] = gateway;
                allFields[gatewayMerchantId.getFieldName()] = gatewayMerchantId;
                allFields[merchantId.getFieldName()] = merchantId;
                allFields[merchantName.getFieldName()] = merchantName;
            }
        };

        FinancialDetailsModel() = default;
        FinancialDetailsModel(const FinancialDetailsModel &) = delete;  // Copy constructor
        FinancialDetailsModel &operator=(const FinancialDetailsModel &) = delete;  // Copy assignment operator
        FinancialDetailsModel(FinancialDetailsModel &&) noexcept = default;  // Move constructor
        FinancialDetailsModel &operator=(FinancialDetailsModel &&) noexcept = default;  // Move assignment operator

        explicit FinancialDetailsModel(const Json::Value &json) : BaseModel(json) {
            taxRate = json[Field::taxRate.getFieldName()].asDouble();
            gateway = json[Field::gateway.getFieldName()].asString();
            gatewayMerchantId = json[Field::gatewayMerchantId.getFieldName()].asString();
            merchantId = json[Field::merchantId.getFieldName()].asString();
            merchantName = json[Field::merchantName.getFieldName()].asString();

            validateField(Field::taxRate.getFieldName(), taxRate, missingFields);
            validateField(Field::gateway.getFieldName(), gateway, missingFields);
            validateField(Field::gatewayMerchantId.getFieldName(), gatewayMerchantId, missingFields);
            validateField(Field::merchantId.getFieldName(), merchantId, missingFields);
            validateField(Field::merchantName.getFieldName(), merchantName, missingFields);
        }

        dec::decimal<2> taxRate;
        std::string gateway;
        std::string gatewayMerchantId;
        std::string merchantId;
        std::string merchantName;
        [[nodiscard]] static std::vector<BaseField> fields();
        [[nodiscard]] std::vector<
            std::pair<BaseField,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
        getObjectValues() const;
    };
}
#endif  //FINANCIALDETAILSMODEL_H
