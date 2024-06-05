#include "Order.h"
#include "src/utils/request/Request.h"
#include "src/orm/QuerySet.h"
#include "src/models/BasketItemModel.h"
#include "src/models/ItemModel.h"
#include "src/models/AddressModel.h"
#include "src/models/UserModel.h"
#include <fmt/core.h>

using namespace api::v1;

void Order::getOneAdmin(const drogon::HttpRequestPtr &req,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                        const std::string &stringId) const {
    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));

    bool isInt = canBeInt(stringId);
    if(auto resp = check404(req, !isInt)) {
        (*callbackPtr)(resp);
        return;
    }

    QuerySet qsUser(UserModel::tableName, "_user");
    qsUser.join(OrderModel())
        .filter(BaseModel<OrderModel>::Field::id.getFullFieldName(), stringId)
        .jsonFields(UserModel().fieldsJsonObject());

    QuerySet qsAddress(AddressModel::tableName, "_address");
    qsAddress.join(OrderModel())
        .filter(BaseModel<OrderModel>::Field::id.getFullFieldName(), stringId)
        .jsonFields(AddressModel().fieldsJsonObject());

    QuerySet qsBasketItem(ItemModel::tableName, 0, std::string("_items"));
    qsBasketItem.join(BasketItemModel())
        .join(OrderModel())
        .filter(BaseModel<OrderModel>::Field::id.getFullFieldName(), stringId)
        .only(ItemModel::Field::title,
              BaseModel<ItemModel>::Field::id,
              BasketItemModel::Field::quantity,
              BasketItemModel::Field::price);

    QuerySet qsOrder(OrderModel::tableName, "_order", true);
    qsOrder.filter(BaseModel<OrderModel>::Field::id.getFullFieldName(), stringId)
        .jsonFields(addExtraQuotes(OrderModel().fieldsJsonObject()))
        .order_by(std::make_pair(BaseModel<OrderModel>::Field::id, false));

    executeSqlQuery(
        callbackPtr,
        QuerySet::buildQuery(std::move(qsOrder), std::move(qsBasketItem), std::move(qsAddress), std::move(qsUser)));
}
