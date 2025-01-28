#include "Auth.h"
#include <drogon/drogon.h>
#include "UserModel.h"
#include <JWT.h>
#include <string>
#include "JwtGoogleFilter.h"

using namespace api::v1;
using namespace drogon::orm;
using namespace api::utils::jwt;

void Auth::googleLogin(const drogon::HttpRequestPtr &request,
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    Json::Value responseJson = *request->getJsonObject();

    std::string credentialsStr = responseJson["credentials"].asString();

    auto [ready, jsonResponse] = filters::JwtGoogleFilter::verifyTokenAndRespond(credentialsStr, callbackPtr);

    if(!ready) {
        return;
    }

    UserModel item(jsonResponse, true);
    if(!item.missingFields.empty()) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(item.missingFields));
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        return (*callbackPtr)(resp);
    }

    executeSqlQuery(callbackPtr, item.sqlGetOrCreateUser());
}

void Auth::googleLoginAdmin(const drogon::HttpRequestPtr &request,
                            std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    Json::Value responseJson = *request->getJsonObject();

    std::string credentialsStr = responseJson["credentials"].asString();

    auto [ready, jsonResponse] = filters::JwtGoogleFilter::verifyTokenAndRespond(credentialsStr, callbackPtr);

    if(!ready) {
        return;
    }
    UserModel item(jsonResponse, true);
    if(!item.missingFields.empty()) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k401Unauthorized);
        return (*callbackPtr)(resp);
    }

    QuerySet qsUser(UserModel::tableName, "_user");
    qsUser.filter(&UserModel::Field::email, item.email)
        .jsonFields(UserModel().fieldsJsonObject())
        .filter(&UserModel::Field::isAdmin, true);
    executeSqlQuery(callbackPtr, qsUser.buildSelect());
}
