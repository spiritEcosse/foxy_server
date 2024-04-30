#include "Auth.h"
#include <drogon/drogon.h>
#include "src/models/UserModel.h"
#include <src/utils/jwt/JWT.h>
#include <json/json.h>
#include <string>

using namespace api::v1;
using namespace drogon::orm;
using namespace api::utils::jwt;

void Auth::getToken(const drogon::HttpRequestPtr &request,
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    Json::Value responseJson = *request->getJsonObject();

    std::string email = responseJson[UserModel::Field::email].asString();
    std::string password = responseJson[UserModel::Field::password].asString();

    if(email.empty() || password.empty()) {
        Json::Value jsonResponse;
        jsonResponse["error"] = "Missing email or password.";
        jsonResponse["status"] = 0;
        auto res = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
        res->setStatusCode(drogon::k400BadRequest);
        (*callbackPtr)(res);
        return;
    }

    std::string query = UserModel::sqlAuth(email);
    auto dbClient = drogon::app().getFastDbClient("default");

    *dbClient << query >> [callbackPtr, responseJson, password, email](const Result &r) {
        if(r.empty()) {
            Json::Value jsonResponse;
            jsonResponse["error"] = "Invalid email or password.";
            jsonResponse["status"] = 0;

            auto res = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
            res->setStatusCode(drogon::k400BadRequest);
            (*callbackPtr)(res);
            return;
        }

        UserModel userModel;
        userModel.password = r[0][UserModel::Field::password].as<std::string>();

        if(!userModel.checkPassword(password)) {
            Json::Value jsonResponse;
            jsonResponse["error"] = "Invalid email or password.";
            jsonResponse["status"] = 0;

            auto res = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
            res->setStatusCode(drogon::k400BadRequest);
            (*callbackPtr)(res);
            return;
        }
        JWT jwtGenerated = JWT::generateToken(
            {
                {"email", picojson::value(email)},
            },
            responseJson.isMember("remember") && responseJson["remember"].asBool());
        std::int64_t jwtExpiration = jwtGenerated.getExpiration();

        Json::Value jsonResponse;
        jsonResponse["token"] = jwtGenerated.getToken();
        jsonResponse["expiresIn"] = jwtExpiration - std::chrono::duration_cast<std::chrono::seconds>(
                                                        std::chrono::system_clock::now().time_since_epoch())
                                                        .count();
        jsonResponse["expiresAt"] = jwtExpiration;
        jsonResponse["status"] = 1;

        auto res = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
        res->setStatusCode(drogon::k200OK);
        (*callbackPtr)(res);
        return;
    } >> [callbackPtr](const DrogonDbException &e) {
        LOG_ERROR << e.base().what();

        auto res = drogon::HttpResponse::newHttpResponse();
        res->setStatusCode(drogon::k500InternalServerError);
        (*callbackPtr)(res);
        return;
    };
}

void Auth::verifyToken(const drogon::HttpRequestPtr &request,
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    Json::Value resultJson;

    resultJson["aud"] = request->getAttributes()->get<std::string>("jwt_aud");
    resultJson["exp"] = request->getAttributes()->get<std::int64_t>("jwt_exp");
    resultJson["iat"] = request->getAttributes()->get<std::int64_t>("jwt_iat");
    resultJson["iss"] = request->getAttributes()->get<std::string>("jwt_iss");
    resultJson["nbf"] = request->getAttributes()->get<std::int64_t>("jwt_nbf");
    resultJson["email"] = request->getAttributes()->get<std::string>("jwt_email");
    resultJson["jwt_debugger"] = "https://jwt.io/#debugger-io?token=" + request->getHeader("Authorization").substr(7);
    resultJson["status"] = 1;

    auto res = drogon::HttpResponse::newHttpJsonResponse(resultJson);
    res->setStatusCode(drogon::k200OK);
    return (*callbackPtr)(res);
}

void Auth::googleLogin(const drogon::HttpRequestPtr &request,
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    Json::Value responseJson = *request->getJsonObject();

    std::string credentialsStr = responseJson["credentials"].asString();

    auto [statusCode, jsonResponse] = JWT::verifyGoogleToken(credentialsStr);
    if(statusCode != drogon::k200OK) {
        Json::Value resultJson;
        resultJson["error"] = jsonResponse["error"];
        auto res = drogon::HttpResponse::newHttpJsonResponse(std::move(resultJson));
        res->setStatusCode(statusCode);
        return (*callbackPtr)(res);
    }
    std::string email = jsonResponse["email"].asString();
    executeSqlQuery(callbackPtr, UserModel::sqlGetOrCreateUser(email));
}
