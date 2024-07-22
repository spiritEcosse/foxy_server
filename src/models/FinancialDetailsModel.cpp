//
// Created by ihor on 07.07.2024.
//

#include "FinancialDetailsModel.h"
#include <fmt/core.h>

using namespace api::v1;

std::vector<
    std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
FinancialDetailsModel::getObjectValues() const {
    return {
        {Field::taxRate, taxRate},
        {Field::gateway, gateway},
        {Field::gatewayMerchantId, gatewayMerchantId},
        {Field::merchantId, merchantId},
        {Field::merchantName, merchantName},
    };
}
