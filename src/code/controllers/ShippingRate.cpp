#include "Item.h"
#include "ShippingRate.h"
#include "Request.h"
#include "QuerySet.h"

using namespace api::v1;
using namespace drogon::orm;

void ShippingRate::getShippingRateByItem(const drogon::HttpRequestPtr &req,
                                         std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                         std::string &&stringId) const {
    const auto callbackPtr =
        std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    const bool isInt = canBeInt(stringId);
    if(const auto resp = check404(req, !isInt && ItemModel::Field::slug.empty())) {
        (*callbackPtr)(resp);
        return;
    }

    const std::string query = ShippingRateModel::getShippingRateByItem(&ItemModel::Field::slug, std::move(stringId));
    executeSqlQuery(callbackPtr, query);
}
