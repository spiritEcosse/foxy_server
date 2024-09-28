#include "BaseCRUD.h"
#include "Request.h"
#include "Item.h"
#include "Page.h"
#include "User.h"
#include "Media.h"
#include "Order.h"
#include "Basket.h"
#include "Tag.h"
#include "Review.h"
#include "Address.h"
#include "BasketItem.h"
#include "ShippingProfile.h"
#include "ShippingRate.h"
#include "Country.h"
#include "SocialMedia.h"
#include "Auth.h"
#include "RequiredFieldsException.h"
#include "FinancialDetails.h"
#include "TagModel.h"
#include "sentryHelper.h"
#include <drogon/utils/Utilities.h>

using namespace api::v1;
using namespace drogon::orm;

template<class T, class R>
std::map<std::string, std::string, std::less<>>
BaseCRUD<T, R>::convertSafeStringMapToStdMap(const drogon::SafeStringMap<std::string> &safeMap) const {
    std::map<std::string, std::string, std::less<>> params;
    for(const auto &[key, value]: safeMap) {
        if(key != "page" && key != "limit" && key != "sort") {
            params[key] = value;
        }
    }
    return params;
}

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

    std::string query = T().sqlDelete(id);
    executeSqlQuery(callbackPtr,
                    query,
                    [this](const drogon::orm::Result &r,
                           std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> _callbackPtr) {
                        this->handleSqlResultDeleting(r, _callbackPtr);
                    });
}

template<class T, class R>
void BaseCRUD<T, R>::deleteItems(const drogon::HttpRequestPtr &req,
                                 std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    if(checkItemsEmpty(req, callbackPtr)) {
        return;
    }
    Json::Value jsonObject = *req->getJsonObject();
    auto itemsJson = jsonObject["items"];

    std::vector<int> ids;
    std::ranges::for_each(itemsJson, [&ids](const auto &item) {
        ids.emplace_back(item.asInt());
    });

    std::string query = T().sqlDeleteMultiple(ids);
    executeSqlQuery(callbackPtr,
                    query,
                    [this](const drogon::orm::Result &r,
                           std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> _callbackPtr) {
                        this->handleSqlResultDeleting(r, _callbackPtr);
                    });
}

template<class T, class R>
void BaseCRUD<T, R>::getItem(const drogon::HttpRequestPtr &req,
                             std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr,
                             std::function<void(T)> successCallback) const {
    if(auto resp = checkBody(req); resp) {
        (*callbackPtr)(resp);
        return;
    }

    Json::Value jsonObject = *req->getJsonObject();
    T item(std::move(jsonObject));
    if(!item.missingFields.empty()) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(item.missingFields));
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        (*callbackPtr)(resp);
        return;
    }
    successCallback(std::move(item));
}

template<class T, class R>
void BaseCRUD<T, R>::getItems(const drogon::HttpRequestPtr &req,
                              std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr,
                              std::function<void(std::vector<T>)> successCallback) const {
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
    std::ranges::for_each(itemsJson, [&items, &index, &jsonResponseError, &req](const auto &item) {
        if(item[T::Field::id.getFieldName()].asInt() == 0 && req->method() == drogon::Put) {
            jsonResponseError[std::to_string(index)] = "id is required";
        }
        items.emplace_back(std::move(item));
        ++index;
    });
    if(!jsonResponseError.empty()) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponseError));
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        (*callbackPtr)(resp);
        return;
    }

    successCallback(std::move(items));
}

template<class T, class R>
void BaseCRUD<T, R>::createItem(const drogon::HttpRequestPtr &req,
                                std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    getItem(req, callbackPtr, [this, callbackPtr](T item) {
        std::string query = T().sqlInsert(item);
        executeSqlQuery(callbackPtr,
                        query,
                        [this](const drogon::orm::Result &r,
                               std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> _callbackPtr) {
                            this->handleSqlResultCreating(r, _callbackPtr);
                        });
    });
}

template<class T, class R>
void BaseCRUD<T, R>::createItems(const drogon::HttpRequestPtr &req,
                                 std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    getItems(req, callbackPtr, [this, callbackPtr](std::vector<T> items) {
        std::string query = T().sqlInsertMultiple(items);
        executeSqlQuery(callbackPtr,
                        query,
                        [this](const drogon::orm::Result &r,
                               std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> _callbackPtr) {
                            this->handleSqlResultItems(r, _callbackPtr, drogon::HttpStatusCode::k201Created);
                        });
    });
}

template<class T, class R>
void BaseCRUD<T, R>::updateItems(const drogon::HttpRequestPtr &req,
                                 std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    getItems(req, callbackPtr, [this, callbackPtr](std::vector<T> items) {
        std::string query = T().sqlUpdateMultiple(items);
        executeSqlQuery(callbackPtr,
                        query,
                        [this](const drogon::orm::Result &r,
                               std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> _callbackPtr) {
                            this->handleSqlResultItems(r, _callbackPtr, drogon::HttpStatusCode::k200OK);
                        });
    });
}

template<class T, class R>
void BaseCRUD<T, R>::getList(const drogon::HttpRequestPtr &req,
                             std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    int page = getInt(req->getParameter("page"), 1);
    int limit = getInt(req->getParameter("limit"), 25);

    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    executeSqlQuery(callbackPtr, T().sqlSelectList(page, limit, convertSafeStringMapToStdMap(req->getParameters())));
}

template<class T, class R>
void BaseCRUD<T, R>::getOne([[maybe_unused]] const drogon::HttpRequestPtr &req,
                            std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                            const std::string &stringId) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    bool isInt = canBeInt(stringId);
    if(auto resp = check404(req, !isInt)) {
        (*callbackPtr)(resp);
        return;
    }

    std::string filterKey = T::Field::id.getFullFieldName();
    std::string query = T().sqlSelectOne(filterKey, stringId, {});

    executeSqlQuery(callbackPtr, query);
}

template<class T, class R>
void BaseCRUD<T, R>::updateItem(const drogon::HttpRequestPtr &req,
                                std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                                const std::string &stringId) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    getItem(req, callbackPtr, [this, callbackPtr, stringId](T item) {
        int id = getInt(stringId, 0);
        if(!id) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::HttpStatusCode::k404NotFound);
            (*callbackPtr)(resp);
            return;
        }
        item.id = id;
        std::string query = T().sqlUpdate(std::move(item));
        executeSqlQuery(callbackPtr, query);
    });
}

template<class T, class R>
void BaseCRUD<T, R>::executeSqlQuery(
    std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr,
    const std::string &query,
    std::function<void(const drogon::orm::Result &,
                       std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>>)> handler) const {
    if(handler == nullptr) {
        handler = [this](const drogon::orm::Result &r,
                         std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> _callbackPtr) {
            this->handleSqlResult(r, _callbackPtr);
        };
    }
    auto dbClient = drogon::app().getFastDbClient("default");
    *dbClient << query >> [callbackPtr, handler](const Result &r) {
        if(r.affectedRows() == 0) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::HttpStatusCode::k404NotFound);
            (*callbackPtr)(resp);
        } else {
            handler(r, callbackPtr);
        }
    } >> [this, callbackPtr](const DrogonDbException &e) {
        this->handleSqlError(e, callbackPtr);
    };
}

template<class T, class R>
void BaseCRUD<T, R>::handleSqlResult(
    const Result &r,
    std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr) const {
    auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(R::getJsonResponse(r)));
    if(r.empty()) {
        resp->setStatusCode(drogon::HttpStatusCode::k404NotFound);
    } else {
        resp->setStatusCode(drogon::HttpStatusCode::k200OK);
    }
    (*callbackPtr)(resp);
}

template<class T, class R>
void BaseCRUD<T, R>::handleSqlResultCreating(
    const Result &r,
    std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr) const {
    auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(R::getJsonResponse(r)));
    resp->setStatusCode(drogon::HttpStatusCode::k201Created);
    (*callbackPtr)(resp);
}

template<class T, class R>
void BaseCRUD<T, R>::handleSqlResultItems(
    const Result &r,
    std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr,
    drogon::HttpStatusCode statusCode) const {
    Json::Value jsonResponse;
    for(const auto &row: r) {
        Json::Value item;
        item = row[0].as<Json::Value>();
        jsonResponse["items"].append(item);
    }
    auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
    resp->setStatusCode(statusCode);
    (*callbackPtr)(resp);
}

template<class T, class R>
void BaseCRUD<T, R>::handleSqlResultDeleting(
    const Result &r,
    std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr) const {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::HttpStatusCode::k204NoContent);
    resp->setContentTypeCode(drogon::ContentType::CT_APPLICATION_JSON);
    (*callbackPtr)(resp);
}

template<class T, class R>
void BaseCRUD<T, R>::handleSqlError(
    const DrogonDbException &e,
    std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr) const {
    std::string errorMsg = e.base().what();
    if(errorMsg.find("duplicate key value violates unique constraint") !=
       std::string::npos) {  // Check if the error is a unique violation
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k409Conflict);
        (*callbackPtr)(resp);
    } else if(errorMsg.find("not_found") != std::string::npos) {  // Check if the error is a not found error
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k404NotFound);
        (*callbackPtr)(resp);
    } else {
        std::string error = e.base().what();
        sentryHelper(error, "handleSqlError");
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
        (*callbackPtr)(resp);
    }
}

template<class T, class R>
Json::Value BaseCRUD<T, R>::getJsonResponse(const Result &r) {
    return r[0][0].as<Json::Value>();
}

template<class T, class R>
bool BaseCRUD<T, R>::checkItemsEmpty(
    const drogon::HttpRequestPtr &req,
    std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> callbackPtr) const {
    if(const Json::Value jsonObject = *req->getJsonObject(); jsonObject["items"].empty()) {
        Json::Value jsonResponse;
        jsonResponse["error"] = "Empty items";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        (*callbackPtr)(resp);
        return true;
    }
    return false;
}

template<class T, class R>
drogon::HttpResponsePtr BaseCRUD<T, R>::checkBody(const drogon::HttpRequestPtr &req) const {
    if(!req->bodyLength()) {
        Json::Value jsonResponse;
        jsonResponse["error"] = "Empty body";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        return resp;
    }
    return nullptr;
}

template<class T, class R>
drogon::HttpResponsePtr BaseCRUD<T, R>::check404(const drogon::HttpRequestPtr &req, bool raise404) const {
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
template class api::v1::BaseCRUD<UserModel, Auth>;
template class api::v1::BaseCRUD<ShippingProfileModel, ShippingProfile>;
template class api::v1::BaseCRUD<CountryModel, Country>;
template class api::v1::BaseCRUD<ShippingRateModel, ShippingRate>;
template class api::v1::BaseCRUD<OrderModel, Order>;
template class api::v1::BaseCRUD<BasketItemModel, BasketItem>;
template class api::v1::BaseCRUD<AddressModel, Address>;
template class api::v1::BaseCRUD<ReviewModel, Review>;
template class api::v1::BaseCRUD<BasketModel, Basket>;
template class api::v1::BaseCRUD<FinancialDetailsModel, FinancialDetails>;
template class api::v1::BaseCRUD<SocialMediaModel, SocialMedia>;
template class api::v1::BaseCRUD<TagModel, Tag>;
