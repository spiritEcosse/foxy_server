#pragma once

#include <decimal.h>
#include "BaseModel.h"

namespace api::v1 {
    class FinancialDetailsModel final : public BaseModel<FinancialDetailsModel> {
    public:
        using BaseModel::BaseModel;

        static const inline std::string tableName = "financial_details";

        struct Field : BaseModel::Field {
            static inline const auto taxRate = BaseField("tax_rate", tableName);
            static inline const auto gateway = BaseField("gateway", tableName);
            static inline const auto gatewayMerchantId = BaseField("gateway_merchant_id", tableName);
            static inline const auto merchantId = BaseField("merchant_id", tableName);
            static inline const auto merchantName = BaseField("merchant_name", tableName);

            Field() : BaseModel::Field() {
                allFields.try_emplace(taxRate.getFieldName(), std::cref(taxRate));
                allFields.try_emplace(gateway.getFieldName(), std::cref(gateway));
                allFields.try_emplace(gatewayMerchantId.getFieldName(), std::cref(gatewayMerchantId));
                allFields.try_emplace(merchantId.getFieldName(), std::cref(merchantId));
                allFields.try_emplace(merchantName.getFieldName(), std::cref(merchantName));
            }
        };

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
        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
    };
}
