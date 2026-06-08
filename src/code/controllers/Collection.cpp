#include "controllers/Collection.h"
#include "utils/request/Request.h"
#include "orm/QuerySet.h"
#include "models/ItemModel.h"
#include "models/CollectionItemModel.h"
#include "utils/config.h"
#include <fmt/core.h>

using namespace api::v1;
using namespace drogon::orm;

void Collection::getOne(const drogon::HttpRequestPtr &req,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                        std::string &&stringId) const {
    const auto callbackPtr =
        std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    // Public access is slug-only: reject numeric ids so collections cannot be probed by sequential id.
    if(const auto resp = check404(req, canBeInt(stringId))) {
        (*callbackPtr)(resp);
        return;
    }

    const std::string query = CollectionModel().sqlSelectOne(&CollectionModel::Field::slug, std::move(stringId), {});
    executeSqlQuery(callbackPtr, query);
}

void Collection::getListAdmin(const drogon::HttpRequestPtr &req,
                              std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    const auto callbackPtr =
        std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    const int page = getInt(req->getParameter("page"), 1);
    int limit = getInt(req->getParameter("limit"), 25);
    auto qsPage = CollectionModel::qsPage(page, limit);

    QuerySet<CollectionModel> qs(limit, "data");
    qs.order_by(&BaseModel<CollectionModel>::Field::updatedAt, false)
        .order_by(&BaseModel<CollectionModel>::Field::id, false)
        .only(CollectionModel::allSetFields())
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit));

    executeSqlQuery(callbackPtr,
                    BuildComplexQueries::buildQuery(CollectionModel::qsCount(), std::move(qsPage), std::move(qs)));
}

void Collection::getOneAdmin(const drogon::HttpRequestPtr &req,
                             std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                             std::string &&stringId) const {
    const auto callbackPtr =
        std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    if(const auto resp = check404(req, !canBeInt(stringId))) {
        (*callbackPtr)(resp);
        return;
    }

    const std::string query =
        CollectionModel().sqlSelectOne(&BaseModel<CollectionModel>::Field::id, std::move(stringId), {});
    executeSqlQuery(callbackPtr, query);
}
