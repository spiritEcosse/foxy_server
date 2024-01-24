#include "Item.h"

using namespace api::v1;
using namespace drogon::orm;

Json::Value Item::getJsonResponse(const Result &r) {
    if (r[0].size() != 2) {
        return BaseCRUD::getJsonResponse(r);
    }
    Json::Value jsonResponse;
    jsonResponse["item"] = r[0][0].as<Json::Value>();
    jsonResponse["media"] = r[0][1].as<Json::Value>();
    return jsonResponse;
}
