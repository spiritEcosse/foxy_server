#include "Item.h"
#include "ShippingRate.h"
#include "Request.h"
#include "QuerySet.h"
#include "env.h"
#include <fmt/core.h>

using namespace api::v1;
using namespace drogon::orm;

void ShippingRate::getShippingRateByItem(const drogon::HttpRequestPtr &req,
                                         std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                         const std::string &stringId) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    bool isInt = canBeInt(stringId);
    if(auto resp = check404(req, !isInt && ItemModel::Field::slug.empty())) {
        (*callbackPtr)(resp);
        return;
    }

    auto ip_address = req->getPeerAddr().toIp();
    std::stringstream ss(ip_address);
    std::string octet;
    std::vector<int> octets;

    while(std::getline(ss, octet, '.')) {
        octets.push_back(std::stoi(octet));
    }

    auto integer_ip = (octets[0] * (256 * 256 * 256)) + (octets[1] * (256 * 256)) + (octets[2] * 256) + octets[3];
    std::map<std::string, std::string, std::less<>> params;
    params["client_ip"] = std::to_string(integer_ip);

    std::string query =
        ShippingRateModel::getShippingRateByItem(ItemModel::Field::slug.getFullFieldName(), stringId, params);

    executeSqlQuery(callbackPtr, query);
}
