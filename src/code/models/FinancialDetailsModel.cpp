#include "FinancialDetailsModel.h"

using namespace api::v1;

BaseModel<FinancialDetailsModel>::SetMapFieldTypes FinancialDetailsModel::getObjectValues() const {
    return {{&Field::taxRate, taxRate},
            {&Field::gateway, gateway},
            {&Field::gatewayMerchantId, gatewayMerchantId},
            {&Field::merchantId, merchantId},
            {&Field::merchantName, merchantName}};
}
