#include "BasketItemModel.h"
#include "ItemModel.h"
#include "MediaModel.h"
#include "config.h"

using namespace api::v1;

BaseModelImpl::JoinMap BasketItemModel::joinMap() {
    return {{ItemModel::tableName, {&Field::itemId, &BaseModel<ItemModel>::Field::id}},
            {OrderModel::tableName, {&Field::basketId, &OrderModel::Field::basketId}}};
}

BaseModel<BasketItemModel>::SetMapFieldTypes BasketItemModel::getObjectValues() const {
    return {{&Field::basketId, basketId},
            {&Field::itemId, itemId},
            {&Field::quantity, quantity},
            {&Field::price, price}};
}

std::string BasketItemModel::sqlSelectList(const int page,
                                           int limit,
                                           const std::map<std::string, std::string, std::less<>> &params) {
    auto qsCount = BasketItemModel::qsCount();
    auto qsPage = BasketItemModel::qsPage(page, limit);

    QuerySet<ItemModel> qsItem("_item", false, false);
    qsItem
        .jsonFields(fmt::format("{}, 'src', format_src(media.src, '{}')",
                                ItemModel().fieldsJsonObject(),
                                getEnv("APP_CLOUD_NAME")))
        .join<MediaModel>()
        .filter(&MediaModel::Field::type, std::string("image"))
        .filter(&BaseModel<ItemModel>::Field::id, &Field::itemId)
        .order_by(&MediaModel::Field::sort);

    QuerySet<BasketItemModel> qs(limit, "data");
    qs.only(allSetFields())
        .join<ItemModel>()
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
        .functions(Function(fmt::format(R"(({}) AS item)", qsItem.buildSelect())));
    applyFilters(qs, qsCount, params);
    return BuildComplexQueries::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}

std::string BasketItemModel::fieldsJsonObject() {
    QuerySet<ItemModel> qs("item", false, false);
    qs.jsonFields(fmt::format("{}, 'src', format_src(media.src, '{}')",
                              ItemModel().fieldsJsonObject(),
                              getEnv("APP_CLOUD_NAME")))
        .filter(&BaseModel<ItemModel>::Field::id, &Field::itemId)
        .filter(&MediaModel::Field::type, std::string("image"))
        .join<MediaModel>()
        .order_by(&MediaModel::Field::sort);
    return fmt::format("{}, 'item', ({})", BaseModel::fieldsJsonObject(), qs.buildSelect());
}
