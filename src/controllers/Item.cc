#include "Item.h"
#include "src/utils/request/Request.h"

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

void Item::getListAdmin(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    auto dbClient = drogon::app().getFastDbClient("default");
    int page = getInt(req->getParameter("page"), 1);
    int limit = getInt(req->getParameter("limit"), 25);

    auto params = req->parameters();
    std::unordered_map<std::string, std::string> paramsMap;
    for(const auto &[key, value]: params) {
        if(key != "page" && key != "limit") {
            paramsMap[key] = value;
        }
    }
    std::string query = ItemModel::BaseModel::sqlSelectList(page, limit, paramsMap);
    *dbClient << query >> [callbackPtr](const Result &r) {
        Json::Value jsonResponse;
        jsonResponse["page"] = r[0][0].as<int>();
        jsonResponse["count"] = r[0][1].as<int>();
        jsonResponse["items"] = r[0][2].as<Json::Value>();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
        resp->addHeader("X-Total-Count", r[0][1].as<std::string>());
        resp->addHeader("Access-Control-Expose-Headers", "X-Total-Count");
        resp->setStatusCode(drogon::HttpStatusCode::k200OK);
        (*callbackPtr)(resp);
    } >> [callbackPtr](const DrogonDbException &e) {
        LOG_ERROR << e.base().what();
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
        (*callbackPtr)(resp);
    };
}
