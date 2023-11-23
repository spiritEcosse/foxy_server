#include "gallery_ItemController.h"
#include "Item.h"
#include "Image.h"

using namespace gallery;
using namespace drogon_model::foxy;
using namespace drogon::orm;

int getInt(const std::string &input, int defaultValue)
{
    try {
        return std::stoi(input);
    }
    catch (const std::exception &e) {
        LOG_ERROR << e.what() << "; input:" << input;
        return defaultValue;
    }
}

// ItemController::get that returns list of items in json format
void
ItemController::get([[maybe_unused]] const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback) const
{
    auto callbackPtr = std::make_shared<std::function<void(const HttpResponsePtr &)>>(std::move(callback));
    auto dbClient = drogon::app().getFastDbClient("default");

    int page = getInt(req->getParameter("page"), 1);
    int limit = 25;

    std::string query = "SELECT "
                        "(SELECT GetValidPage($1, $2)) as page,\n"
                        "(SELECT count(*) FROM "
        + Item::tableName + ") as count,\n"
                            "(SELECT json_agg(t.*) FROM (\n"
                            "    SELECT t1.*, t2.original FROM " + Item::tableName + " as t1 NATURAL JOIN "
        + Image::tableName + " as t2\n"
                             "    ORDER BY t1.id\n"
                             "    OFFSET (GetValidPage($1, $2) - 1) * $2\n"
                             "    LIMIT $2) as t) AS items;";
    *dbClient << query
              << page
              << limit
              >> [callbackPtr](const Result &r)
              {
                  // Create a JSON response
                  Json::Value jsonResponse;

                  jsonResponse["page"] = r[0][0].as<int>();
                  jsonResponse["count"] = r[0][1].as<int>();
                  jsonResponse["items"] = r[0][2].as<Json::Value>();
                  auto resp = HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
                  resp->setStatusCode(HttpStatusCode::k200OK);
                  (*callbackPtr)(resp);
              }
              >> [callbackPtr](const DrogonDbException &e)
              {
                  LOG_ERROR << e.base().what();
                  auto resp = HttpResponse::newHttpResponse();
                  resp->setStatusCode(HttpStatusCode::k500InternalServerError);
                  (*callbackPtr)(resp);
              };
}

// ItemController::getItem that returns object of item in json format
void ItemController::getItem([[maybe_unused]] const HttpRequestPtr &req,
                             std::function<void(const HttpResponsePtr &)> &&callback,
                             int id) const
{
    auto callbackPtr = std::make_shared<std::function<void(const HttpResponsePtr &)>>(std::move(callback));
    auto dbClient = drogon::app().getFastDbClient("default");
    std::string query = "SELECT "
                        "(SELECT json_build_object('id', id,\n"
                        "                    'title', title,\n"
                        "                    'description', description,\n"
                        "                    'meta_description', meta_description\n"
                        ") FROM " + Item::tableName
        + " where id = $1) as item,\n"
          "(SELECT json_agg(t2.*) from (Select original from " + Image::tableName
        + " where item_id = $1) as t2) as images";

    *dbClient << query
              << std::to_string(id)
              >> [callbackPtr](const Result &r)
              {
                  if (r[0][0].isNull()) {
                      auto resp = HttpResponse::newHttpResponse();
                      resp->setStatusCode(HttpStatusCode::k404NotFound);
                      (*callbackPtr)(resp);
                      return;
                  }
                  // Create a JSON response
                  Json::Value jsonResponse;
                  jsonResponse["item"] = r[0][0].as<Json::Value>();
                  jsonResponse["images"] = r[0][1].as<Json::Value>();
                  auto resp = HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
                  resp->setStatusCode(HttpStatusCode::k200OK);
                  (*callbackPtr)(resp);
              } >> [callbackPtr](const DrogonDbException &e)
              {
                  LOG_ERROR << e.base().what();
                  auto resp = HttpResponse::newHttpResponse();
                  resp->setStatusCode(HttpStatusCode::k500InternalServerError);
                  (*callbackPtr)(resp);
              };
}
