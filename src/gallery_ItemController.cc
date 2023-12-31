#include "gallery_ItemController.h"

using namespace gallery;
using namespace drogon::orm;

int getInt(const std::string &input, int defaultValue) {
    if(input.empty()) {
        return defaultValue;
    }
    try {
        return std::stoi(input);
    } catch(const std::invalid_argument &e) {
        LOG_ERROR << e.what() << "; input:" << input;
        return defaultValue;
    }
}

// ItemController::get that returns list of items in json format
void ItemController::get([[maybe_unused]] const drogon::HttpRequestPtr &req,
                         std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    auto dbClient = drogon::app().getFastDbClient("default");

    int page = getInt(req->getParameter("page"), 1);
    int limit = 25;

    std::string query = "SELECT "
                        "(SELECT GetValidPage($1, $2)) as page,\n"
                        "(SELECT count(*) FROM item) as count,\n"
                        "(SELECT json_agg(t.*) FROM (\n"
                        "    SELECT DISTINCT t1.*, t2.src FROM item as t1 NATURAL JOIN media as t2\n"
                        "    WHERE t2.sort = 1\n"
                        "    ORDER BY t1.item_id\n"
                        "    OFFSET (GetValidPage($1, $2) - 1) * $2\n"
                        "    LIMIT $2) as t) AS items;";

    *dbClient << query << page << limit >> [callbackPtr](const Result &r) {
        // Create a JSON response
        Json::Value jsonResponse;

        jsonResponse["page"] = r[0][0].as<int>();
        jsonResponse["count"] = r[0][1].as<int>();
        jsonResponse["items"] = r[0][2].as<Json::Value>();
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

// ItemController::getItem that returns object of item in json format
void ItemController::getItem([[maybe_unused]] const drogon::HttpRequestPtr &req,
                             std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                             const std::string &stringId) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    auto dbClient = drogon::app().getFastDbClient("default");
    std::string query = "SELECT "
                        "(SELECT json_build_object('item_id', item_id,\n"
                        "                    'title', title,\n"
                        "                    'description', description,\n"
                        "                    'meta_description', meta_description\n"
                        ") FROM item where item_id = $1) as item,\n"
                        "(SELECT json_agg(t2.*) from (Select src, thumb, media_id from media where item_id = $1 order "
                        "by sort) as t2) as media";

    int id = getInt(stringId, 0);
    if(id == 0) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k404NotFound);
        (*callbackPtr)(resp);
        return;
    }

    *dbClient << query << id >> [callbackPtr](const Result &r) {
        if(r[0][0].isNull()) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::HttpStatusCode::k404NotFound);
            (*callbackPtr)(resp);
            return;
        }
        // Create a JSON response
        Json::Value jsonResponse;
        jsonResponse["item"] = r[0][0].as<Json::Value>();
        jsonResponse["media"] = r[0][1].as<Json::Value>();
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
