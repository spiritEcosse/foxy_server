#include "BaseCRUD.h"
#include "src/utils/request/Request.h"
#include "src/controllers/Item.h"
#include "src/controllers/Page.h"
#include "src/controllers/User.h"
#include "src/controllers/Media.h"

using namespace api::v1;
using namespace drogon::orm;

template<class T, class R>
void BaseCRUD<T, R>::deleteItem(const drogon::HttpRequestPtr &req,
                                std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                const std::string &stringId) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    auto dbClient = drogon::app().getFastDbClient("default");
    int id = getInt(stringId, 0);
    if(id == 0) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k404NotFound);
        (*callbackPtr)(resp);
        return;
    }

    std::string query = T::sqlDelete(id);

    *dbClient << query >> [callbackPtr]([[maybe_unused]] const Result &r) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k204NoContent);
        (*callbackPtr)(resp);
    } >> [callbackPtr](const DrogonDbException &e) {
        LOG_ERROR << e.base().what();
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
        (*callbackPtr)(resp);
    };
}

template<class T, class R>
void BaseCRUD<T, R>::createItem(const drogon::HttpRequestPtr &req,
                                std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    if(auto resp = checkBody(req); resp) {
        (*callbackPtr)(resp);
        return;
    }

    Json::Value jsonObject = *req->getJsonObject();
    Json::Value jsonResponseError;
    T item;
    try {
        item = T(std::move(jsonObject));
    } catch(const RequiredFieldsException &e) {
        jsonResponseError = e.getRequiredFields();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponseError));
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        (*callbackPtr)(resp);
        return;
    }
    std::string query = T::sqlInsert(item);
    auto dbClient = drogon::app().getFastDbClient("default");
    *dbClient << query >> [callbackPtr](const Result &r) {
        if(r.empty()) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::HttpStatusCode::k409Conflict);
            (*callbackPtr)(resp);
            return;
        }
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(R::getJsonResponse(r)));
        resp->setStatusCode(drogon::HttpStatusCode::k201Created);
        (*callbackPtr)(resp);
    } >> [callbackPtr](const DrogonDbException &e) {
        LOG_ERROR << e.base().what();
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
        (*callbackPtr)(resp);
    };
}

template<class T, class R>
void BaseCRUD<T, R>::createItems(const drogon::HttpRequestPtr &req,
                                 std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    if(auto resp = checkBody(req); resp) {
        (*callbackPtr)(resp);
        return;
    }

    Json::Value jsonObject = *req->getJsonObject();
    auto itemsJson = jsonObject["items"];
    if(itemsJson.empty()) {
        Json::Value jsonResponse;
        jsonResponse["error"] = "Empty items";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        (*callbackPtr)(resp);
        return;
    }

    std::vector<T> items;
    int index = 1;
    Json::Value jsonResponseError;
    try {
        std::for_each(itemsJson.begin(), itemsJson.end(), [&items, &index](const auto &item) {
            items.emplace_back(std::move(item));
            ++index;
        });
    } catch(const RequiredFieldsException &e) {
        jsonResponseError[std::to_string(index)] = e.getRequiredFields();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponseError));
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        (*callbackPtr)(resp);
        return;
    }
    std::string query = T::sqlInsertMultiple(items);
    auto dbClient = drogon::app().getFastDbClient("default");
    *dbClient << query >> [callbackPtr](const Result &r) {
        Json::Value jsonResponse;
        for(const auto &row: r) {
            Json::Value item;
            item = row[0].as<Json::Value>();
            jsonResponse["items"].append(item);
        }
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
        resp->setStatusCode(drogon::HttpStatusCode::k201Created);
        (*callbackPtr)(resp);
    } >> [callbackPtr](const DrogonDbException &e) {
        LOG_ERROR << e.base().what();
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
        (*callbackPtr)(resp);
    };
}

template<class T, class R>
void BaseCRUD<T, R>::getList(const drogon::HttpRequestPtr &req,
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
    std::string query = T::sqlSelectList(page, limit, paramsMap);
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

template<class T, class R>
void BaseCRUD<T, R>::getOne([[maybe_unused]] const drogon::HttpRequestPtr &req,
                            std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                            const std::string &stringId) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    auto dbClient = drogon::app().getFastDbClient("default");

    bool isInt = canBeInt(stringId);
    if(!isInt && T::Field::slug.empty()) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k404NotFound);
        (*callbackPtr)(resp);
        return;
    }

    std::string filterKey = isInt ? T::primaryKey : T::Field::slug;
    std::string query = T::sqlSelectOne(filterKey, stringId);

    *dbClient << query >> [callbackPtr](const Result &r) {
        if(r[0][0].isNull()) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::HttpStatusCode::k404NotFound);
            (*callbackPtr)(resp);
            return;
        }
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(R::getJsonResponse(r)));
        resp->setStatusCode(drogon::HttpStatusCode::k200OK);
        (*callbackPtr)(resp);
    } >> [callbackPtr](const DrogonDbException &e) {
        LOG_ERROR << e.base().what();
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
        (*callbackPtr)(resp);
    };
}

template<class T, class R>
void BaseCRUD<T, R>::updateItem(const drogon::HttpRequestPtr &req,
                                std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                const std::string &stringId) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    if(auto resp = checkBody(req); resp) {
        (*callbackPtr)(resp);
        return;
    }

    Json::Value jsonObject = *req->getJsonObject();
    Json::Value jsonResponseError;
    T item;
    try {
        item = T(std::move(jsonObject));
    } catch(const RequiredFieldsException &e) {
        jsonResponseError = e.getRequiredFields();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponseError));
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        (*callbackPtr)(resp);
        return;
    }
    int id = getInt(stringId, 0);
    if(!id) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k404NotFound);
        (*callbackPtr)(resp);
        return;
    }
    item.id = id;

    std::string query = T::sqlUpdate(item);

    auto dbClient = drogon::app().getFastDbClient("default");
    *dbClient << query >> [callbackPtr](const Result &r) {
        if(r.empty()) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::HttpStatusCode::k404NotFound);
            (*callbackPtr)(resp);
            return;
        }
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(R::getJsonResponse(r)));
        resp->setStatusCode(drogon::HttpStatusCode::k200OK);
        (*callbackPtr)(resp);
    } >> [callbackPtr](const DrogonDbException &e) {
        LOG_ERROR << e.base().what();
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
        (*callbackPtr)(resp);
    };
}

template<class T, class R>
Json::Value BaseCRUD<T, R>::getJsonResponse(const Result &r) {
    Json::Value jsonResponse;
    jsonResponse = r[0][0].as<Json::Value>();
    return jsonResponse;
}

template <class T, class R>
drogon::HttpResponsePtr BaseCRUD<T, R>::checkBody(const drogon::HttpRequestPtr& req) const {
    if (!req->bodyLength()) {
        Json::Value jsonResponse;
        jsonResponse["error"] = "Empty body";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        return resp;
    }
    return nullptr;
}

template class api::v1::BaseCRUD<ItemModel, Item>;
template class api::v1::BaseCRUD<PageModel, Page>;
template class api::v1::BaseCRUD<UserModel, User>;
template class api::v1::BaseCRUD<MediaModel, Media>;
