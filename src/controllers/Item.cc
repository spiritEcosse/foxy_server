#include "Item.h"
#include "src/utils/request/Request.h"
#include "src/orm/QuerySet.h"
#include "src/models/MediaModel.h"

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

    QuerySet qs(ItemModel::tableName);
    qs.distinct({ItemModel::tableName + "." + ItemModel::orderBy, ItemModel::tableName + "." + ItemModel::Field::id})
        .left_join(MediaModel::tableName, ItemModel::tableName + "." + ItemModel::Field::id + " = " + MediaModel::tableName + "." + MediaModel::Field::itemId)
        .order_by({{ItemModel::tableName + "." + ItemModel::orderBy, false}, {ItemModel::tableName + "." + ItemModel::Field::id, false}})
        .limit(limit)
        .only({ItemModel::fullFieldsWithTableToString(), MediaModel::tableName + "." + MediaModel::Field::src})
        .page(page);
    *dbClient << qs.buildSelect() >> [callbackPtr](const Result &r) {
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
