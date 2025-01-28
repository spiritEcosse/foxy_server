#include "BasketItemModel.h"
#include "ItemModel.h"
#include "MediaModel.h"
#include "env.h"

using namespace api::v1;

BaseModelImpl::JoinMap BasketItemModel::joinMap() const {
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
    QuerySet qsCount = BasketItemModel::qsCount();
    QuerySet qsPage = BasketItemModel::qsPage(page, limit);

    QuerySet qsItem(ItemModel::tableName, "_item", false, false);
    qsItem
        .jsonFields(
            fmt::format("{}, 'src', format_src(media.src, '{}')", ItemModel().fieldsJsonObject(), APP_CLOUD_NAME))
        .join(MediaModel())
        .join(BasketItemModel())
        .order_by(&MediaModel::Field::sort);

    QuerySet qs(tableName, limit, "data");
    qs.only(allSetFields())
        .join(ItemModel())
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
        .functions(Function(fmt::format(R"(({}) AS item)", qsItem.buildSelect())));
    applyFilters(qs, qsCount, params);
    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}

std::string BasketItemModel::fieldsJsonObject() {
    std::string str = BaseModel::fieldsJsonObject();
    QuerySet qs(ItemModel::tableName, "item", false, false);
    qs.jsonFields(fmt::format("{}, 'src', format_src(media.src, '{}')", ItemModel().fieldsJsonObject(), APP_CLOUD_NAME))
        .join(BasketItemModel())
        .join(MediaModel())
        .order_by(&MediaModel::Field::sort);
    return fmt::format(", 'item', ({})", qs.buildSelect());
}
