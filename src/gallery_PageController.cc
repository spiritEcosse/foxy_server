#include "gallery_PageController.h"

using namespace gallery;
using namespace drogon::orm;

// ItemController::getItem that returns object of item in json format
void PageController::getItem([[maybe_unused]] const drogon::HttpRequestPtr &req,
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

// Add definition of your processing function here
