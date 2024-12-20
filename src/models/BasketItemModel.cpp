#include "BasketItemModel.h"
#include "ItemModel.h"
#include "MediaModel.h"
#include "env.h"

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> BasketItemModel::joinMap() const {
    return {
        {ItemModel::tableName, {Field::itemId.getFullFieldName(), BaseModel<ItemModel>::Field::id.getFullFieldName()}},
        {OrderModel::tableName, {Field::basketId.getFullFieldName(), OrderModel::Field::basketId.getFullFieldName()}}};
}

BaseModel<BasketItemModel>::SetMapFieldTypes BasketItemModel::getObjectValues() const {
    return {{std::cref(Field::basketId), std::cref(basketId)},
            {std::cref(Field::itemId), std::cref(itemId)},
            {std::cref(Field::quantity), std::cref(quantity)},
            {std::cref(Field::price), std::cref(price)}};
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
        .filter(BaseModel<ItemModel>::Field::id.getFullFieldName(),
                Field::itemId.getFullFieldName(),
                false,
                std::string("="))
        .order_by(std::make_pair(std::cref(MediaModel::Field::sort), true));

    QuerySet qs(tableName, limit, "data");
    qs.only(allSetFields())
        .join(ItemModel())
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
        .functions(Function(fmt::format(R"(, ({}) AS item)", qsItem.buildSelect())));
    applyFilters(qs, qsCount, params);
    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}

std::string BasketItemModel::fieldsJsonObject() {
    std::string str = BaseModel::fieldsJsonObject();
    QuerySet qs(ItemModel::tableName, "item", false, false);
    qs.jsonFields(fmt::format("{}, 'src', format_src(media.src, '{}')", ItemModel().fieldsJsonObject(), APP_CLOUD_NAME))
        .filter(BaseModel<ItemModel>::Field::id.getFullFieldName(),
                Field::itemId.getFullFieldName(),
                false,
                std::string("="))
        .join(MediaModel())
        .order_by(std::make_pair(std::cref(MediaModel::Field::sort), true));
    std::string sql = qs.buildSelect();
    str += fmt::format(", 'item', ({})", sql);
    return str;
}
