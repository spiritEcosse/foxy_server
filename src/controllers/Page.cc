#include "Page.h"
#include "src/utils/request/Request.h"

using namespace api::v1;
using namespace drogon::orm;

// ItemController::getItem that returns object of item in json format
void Page::getItem([[maybe_unused]] const drogon::HttpRequestPtr &req,
                   std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                   std::string canonical_url) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    auto dbClient = drogon::app().getFastDbClient("default");
    std::string query = "SELECT (SELECT json_build_object('page_id', page_id,\n"
                        "                    'title', title,\n"
                        "                    'description', description,\n"
                        "                    'meta_description', meta_description,\n"
                        "                    'canonical_url', canonical_url\n"
                        ") FROM page where canonical_url = $1 ) as page ";

    *dbClient << query << canonical_url >> [callbackPtr](const Result &r) {
        if(r[0][0].isNull()) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::HttpStatusCode::k404NotFound);
            (*callbackPtr)(resp);
            return;
        }
        // Create a JSON response
        Json::Value jsonResponse;
        jsonResponse = r[0][0].as<Json::Value>();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
        resp->setStatusCode(drogon::HttpStatusCode::k200OK);
        (*callbackPtr)(resp);
    } >> [callbackPtr](const DrogonDbException &e) {
        LOG_ERROR << e.base().what();
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
        (*callbackPtr)(resp);
    };
}

void Page::createItem(const drogon::HttpRequestPtr &req,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    if(!req->bodyLength()) {
        Json::Value jsonResponse;
        jsonResponse["error"] = "Empty body";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        (*callbackPtr)(resp);
        return;
    }

    Json::Value jsonObject = *req->getJsonObject();
    if(!jsonObject.isMember("title") || !jsonObject.isMember("description") ||
        !jsonObject.isMember("meta_description")) {
        Json::Value jsonResponse;
        jsonResponse["error"] = "Missing fields";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        (*callbackPtr)(resp);
        return;
    }

    auto dbClient = drogon::app().getFastDbClient("default");
    std::string query = "INSERT INTO item (title, description, meta_description) VALUES ($1, $2, $3) RETURNING json_build_object('item_id', item_id,\n"
                        "                                            'title', title,\n"
                        "                                            'description', description,\n"
                        "                                            'meta_description', meta_description\n"
                        ")";

    *dbClient << query << jsonObject["title"].asString() << jsonObject["description"].asString()
              << jsonObject["meta_description"].asString() >>
              [callbackPtr](const Result &r) {
                  Json::Value jsonResponse;
                  jsonResponse["item"] = r[0][0].as<Json::Value>();
                  auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
                  resp->setStatusCode(drogon::HttpStatusCode::k201Created);
                  (*callbackPtr)(resp);
              } >>
              [callbackPtr](const DrogonDbException &e) {
                  LOG_ERROR << e.base().what();
                  auto resp = drogon::HttpResponse::newHttpResponse();
                  resp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
                  (*callbackPtr)(resp);
              };
}

void Page::updateItem(const drogon::HttpRequestPtr &req,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                      const std::string &stringId) {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    if(!req->bodyLength()) {
        Json::Value jsonResponse;
        jsonResponse["error"] = "Empty body";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        (*callbackPtr)(resp);
        return;
    }

    Json::Value jsonObject = *req->getJsonObject();
    if(!jsonObject.isMember("title") || !jsonObject.isMember("description") ||
        !jsonObject.isMember("meta_description")) {
        Json::Value jsonResponse;
        jsonResponse["error"] = "Missing fields";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        (*callbackPtr)(resp);
        return;
    }

    int id = getInt(stringId, 0);
    if(id == 0) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k404NotFound);
        (*callbackPtr)(resp);
        return;
    }

    auto dbClient = drogon::app().getFastDbClient("default");
    std::string query = "UPDATE page SET title = $1, description = $2, meta_description = $3 WHERE item_id = $4 RETURNING json_build_object('item_id', item_id,\n"
                        "                                            'title', title,\n"
                        "                                            'description', description,\n"
                        "                                            'meta_description', meta_description\n"
                        ")";

    *dbClient << query << jsonObject["title"].asString() << jsonObject["description"].asString()
              << jsonObject["meta_description"].asString() << id >>
              [callbackPtr](const Result &r) {
                  Json::Value jsonResponse;
                  jsonResponse["item"] = r[0][0].as<Json::Value>();
                  auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
                  resp->setStatusCode(drogon::HttpStatusCode::k200OK);
                  (*callbackPtr)(resp);
              } >>
              [callbackPtr](const DrogonDbException &e) {
                  LOG_ERROR << e.base().what();
                  auto resp = drogon::HttpResponse::newHttpResponse();
                  resp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
                  (*callbackPtr)(resp);
              };
}


// Add definition of your processing function here
