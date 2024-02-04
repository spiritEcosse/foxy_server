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
    int id = getInt(stringId, 0);
    if(auto resp = check404(req, !id)) {
        (*callbackPtr)(resp);
        return;
    }

    std::string query = T::sqlDelete(id);
    executeSqlQuery(callbackPtr, query,
                    [this](const drogon::orm::Result &r, std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> _callbackPtr) {
                        this->handleSqlResultDeleting(r, _callbackPtr);
                    });
}

template<class T, class R>
void BaseCRUD<T, R>::getItem(
    const drogon::HttpRequestPtr &req, std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr,
    std::function<void(T)> successCallback) const {
    if(auto resp = checkBody(req); resp) {
        (*callbackPtr)(resp);
        return;
    }

    Json::Value jsonObject = *req->getJsonObject();
    T item;
    try {
        item = T(std::move(jsonObject));
    } catch([[maybe_unused]] const RequiredFieldsException &e) {
        Json::Value jsonResponseError;
        jsonResponseError = e.getRequiredFields();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponseError));
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        (*callbackPtr)(resp);
        return;
    }
    successCallback(std::move(item));
}

template<class T, class R>
void BaseCRUD<T, R>::createItem(const drogon::HttpRequestPtr &req,
                                std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    getItem(req, callbackPtr, [this, callbackPtr](T item)
    {
        std::string query = T::sqlInsert(item);
        executeSqlQuery(callbackPtr, query,
                        [this](const drogon::orm::Result &r, std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> _callbackPtr) {
                            this->handleSqlResultCreating(r, _callbackPtr);
                        });
    });
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
        std::ranges::for_each(itemsJson.begin(), itemsJson.end(), [&items, &index](const auto &item) {
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
    executeSqlQuery(callbackPtr, query,
                    [this](const drogon::orm::Result &r, std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> _callbackPtr) {
                        this->handleSqlResultCreatingItems(r, _callbackPtr);
                    });
}

template<class T, class R>
void BaseCRUD<T, R>::getList(const drogon::HttpRequestPtr &req,
                             std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    int page = getInt(req->getParameter("page"), 1);
    int limit = getInt(req->getParameter("limit"), 25);
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    executeSqlQuery(callbackPtr, T::sqlSelectList(page, limit),
                    [this](const drogon::orm::Result &r, std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> _callbackPtr) {
                        this->handleSqlResultList(r, _callbackPtr);
                    });
}

template<class T, class R>
void BaseCRUD<T, R>::getOne([[maybe_unused]] const drogon::HttpRequestPtr &req,
                            std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                            const std::string &stringId) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    bool isInt = canBeInt(stringId);
    if(auto resp = check404(req, !isInt && T::Field::slug.empty())) {
        (*callbackPtr)(resp);
        return;
    }

    std::string filterKey = isInt ? T::primaryKey : T::Field::slug;
    std::string query = T::sqlSelectOne(filterKey, stringId);

    executeSqlQuery(callbackPtr, query);
}

template<class T, class R>
void BaseCRUD<T, R>::updateItem(const drogon::HttpRequestPtr &req,
                                std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                const std::string &stringId) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    getItem(req, callbackPtr, [this, callbackPtr, stringId](T item)
    {
        int id = getInt(stringId, 0);
        if(!id) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::HttpStatusCode::k404NotFound);
            (*callbackPtr)(resp);
            return;
        }
        item.id = id;
        std::string query = T::sqlUpdate(item);
        executeSqlQuery(callbackPtr, query);
    });
}

template<class T, class R>
void BaseCRUD<T, R>::executeSqlQuery(
    std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr,
    const std::string& query,
    std::function<void(const drogon::orm::Result &, std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>>)> handler) const {
    if (handler == nullptr) {
        handler = [this](const drogon::orm::Result &r, std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> _callbackPtr) {
            this->handleSqlResult(r, _callbackPtr);
        };
    }
    auto dbClient = drogon::app().getFastDbClient("default");
    *dbClient << query >> [callbackPtr, handler](const Result &r) {
        // Call the handler function
        handler(r, callbackPtr);
    } >> [this, callbackPtr](const DrogonDbException &e) {
        this->handleSqlError(e, callbackPtr);
    };
}

template<class T, class R>
void BaseCRUD<T, R>::handleSqlResultList(const Result &r, std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr) const {
    Json::Value jsonResponse;
    jsonResponse["page"] = r[0][0].as<int>();
    jsonResponse["total"] = r[0][1].as<int>();
    jsonResponse["data"] = r[0][2].as<Json::Value>();
    auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
    resp->addHeader("X-Total-Count", r[0][1].as<std::string>());
    resp->addHeader("Access-Control-Expose-Headers", "X-Total-Count");
    resp->setStatusCode(drogon::HttpStatusCode::k200OK);
    (*callbackPtr)(resp);
}

template<class T, class R>
void BaseCRUD<T, R>::handleSqlResult(const Result &r, std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr) const {
    auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(R::getJsonResponse(r)));
    resp->setStatusCode(drogon::HttpStatusCode::k200OK);
    (*callbackPtr)(resp);
}

template<class T, class R>
void BaseCRUD<T, R>::handleSqlResultCreating(const Result &r, std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr) const {
    auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(R::getJsonResponse(r)));
    resp->setStatusCode(drogon::HttpStatusCode::k201Created);
    (*callbackPtr)(resp);
}

template<class T, class R>
void BaseCRUD<T, R>::handleSqlResultCreatingItems(const Result &r, std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr) const {
    Json::Value jsonResponse;
    for(const auto &row: r) {
        Json::Value item;
        item = row[0].as<Json::Value>();
        jsonResponse["items"].append(item);
    }
    auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
    resp->setStatusCode(drogon::HttpStatusCode::k201Created);
    (*callbackPtr)(resp);
}

template<class T, class R>
void BaseCRUD<T, R>::handleSqlResultDeleting(const Result &r, std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr) const {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::HttpStatusCode::k204NoContent);
    (*callbackPtr)(resp);
}

template<class T, class R>
void BaseCRUD<T, R>::handleSqlError(const DrogonDbException &e, std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr) const {
    LOG_ERROR << e.base().what();
    std::string errorMsg = e.base().what();
    if (errorMsg.find("duplicate key value violates unique constraint") != std::string::npos) { // Check if the error is a unique violation
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k409Conflict);
        (*callbackPtr)(resp);
    } else if (errorMsg.find("not_found") != std::string::npos) { // Check if the error is a not found error
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k404NotFound);
        (*callbackPtr)(resp);
    } else {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
        (*callbackPtr)(resp);
    }
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

template <class T, class R>
drogon::HttpResponsePtr BaseCRUD<T, R>::check404(const drogon::HttpRequestPtr& req, bool raise404) const {
    if(raise404) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k404NotFound);
        return resp;
    }
    return nullptr;
}

template class api::v1::BaseCRUD<ItemModel, Item>;
template class api::v1::BaseCRUD<PageModel, Page>;
template class api::v1::BaseCRUD<UserModel, User>;
template class api::v1::BaseCRUD<MediaModel, Media>;
