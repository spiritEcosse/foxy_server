#include "FinancialDetailsModel.h"

using namespace api::v1;

BaseModel<FinancialDetailsModel>::SetMapFieldTypes FinancialDetailsModel::getObjectValues() const {
    return {
        {std::cref(Field::taxRate), std::cref(taxRate)},
        {std::cref(Field::gateway), std::cref(gateway)},
        {std::cref(Field::gatewayMerchantId), std::cref(gatewayMerchantId)},
        {std::cref(Field::merchantId), std::cref(merchantId)},
        {std::cref(Field::merchantName), std::cref(merchantName)},
    };
}
