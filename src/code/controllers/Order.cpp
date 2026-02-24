#include "controllers/Order.h"
#include "utils/request/Request.h"
#include "orm/QuerySet.h"
#include "models/BasketItemModel.h"
#include "models/ItemModel.h"
#include "models/AddressModel.h"
#include "models/UserModel.h"
#include <fmt/core.h>

using namespace api::v1;

void Order::getOneAdmin(const drogon::HttpRequestPtr &req,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                        std::string &&stringId) const {
    const auto callbackPtr =
        std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    const bool isInt = canBeInt(stringId);
    if(const auto resp = check404(req, !isInt)) {
        (*callbackPtr)(resp);
        return;
    }

    QuerySet<UserModel> qsUser(0, UserModel::tableName, false);
    qsUser.filter(&BaseModel<UserModel>::Field::id, &OrderModel::Field::userId)
        .jsonFields(UserModel().fieldsJsonObject());

    QuerySet<AddressModel> qsAddress(0, AddressModel::tableName, false);
    qsAddress.filter(&BaseModel<AddressModel>::Field::id, &OrderModel::Field::addressId)
        .jsonFields(AddressModel().fieldsJsonObject());

    QuerySet<ItemModel> qsItems(0, ItemModel::tableName, false);
    qsItems.join<BasketItemModel>()
        .filter(&BasketItemModel::Field::basketId, &OrderModel::Field::basketId)
        .functions(Function(fmt::format("json_agg(json_build_object({}))", BasketItemModel().fieldsJsonObject())));

    QuerySet<OrderModel> qsOrder(OrderModel::tableName, true, true);
    qsOrder.filter(&BaseModel<OrderModel>::Field::id, std::move(stringId))
        .jsonFields(addExtraQuotes(OrderModel().fieldsJsonObject()))
        .functions(Function(addExtraQuotes(fmt::format(R"(
             'items', COALESCE(({}),'[]'::json),
             'user', ({}),
             'address', ({})
         )",
                                                       qsItems.buildSelect(),
                                                       qsUser.buildSelectOne(),
                                                       qsAddress.buildSelectOne()))))
        .order_by(&BaseModel<OrderModel>::Field::id, false);

    executeSqlQuery(callbackPtr, qsOrder.buildSelectOne());
}

void Order::getListAdmin(const drogon::HttpRequestPtr &req,
                         std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    int page = getInt(req->getParameter("page"), 1);
    int limit = getInt(req->getParameter("limit"), 25);

    auto qsCount = OrderModel::qsCount();
    auto qsPage = OrderModel::qsPage(page, limit);

    QuerySet<OrderModel> qs(limit, "data");
    qs.left_join<BasketItemModel>()
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
        .only(OrderModel::allSetFields())
        .functions(Function(
            fmt::format(R"(COUNT({}) as count_items)", BaseModel<BasketItemModel>::Field::id.getFullFieldName())))
        .order_by(&BaseModel<OrderModel>::Field::updatedAt, false)
        .order_by(&BaseModel<OrderModel>::Field::id, false)
        .group_by(&BaseModel<OrderModel>::Field::id, &BaseModel<OrderModel>::Field::updatedAt);
    const auto params = BaseCRUD().convertSafeStringMapToStdMap(req->getParameters());
    OrderModel::applyFilters(qs, qsCount, params);
    executeSqlQuery(callbackPtr, BuildComplexQueries::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs)));
}
