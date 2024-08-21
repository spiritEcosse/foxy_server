//
// Created by ihor on 14.01.2024.
//

#include "BasketItemModel.h"
#include "ItemModel.h"
#include "MediaModel.h"
#include "env.h"

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> BasketItemModel::joinMap() const {
    return {{ItemModel::tableName,
             {BasketItemModel::Field::itemId.getFullFieldName(), BaseModel<ItemModel>::Field::id.getFullFieldName()}},
            {OrderModel::tableName,
             {BasketItemModel::Field::basketId.getFullFieldName(), OrderModel::Field::basketId.getFullFieldName()}}};
}

std::vector<
    std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
BasketItemModel::getObjectValues() const {
    return {{Field::basketId, basketId}, {Field::itemId, itemId}, {Field::quantity, quantity}, {Field::price, price}};
};

std::string
BasketItemModel::sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params) {
    QuerySet qsCount = BasketItemModel().qsCount();
    QuerySet qsPage = BasketItemModel().qsPage(page, limit);

    std::string app_cloud_name;
    getenv("APP_CLOUD_NAME", app_cloud_name);

    QuerySet qsItem(ItemModel::tableName, "_item", false, false);
    qsItem
        .jsonFields(
            fmt::format("{}, 'src', format_src(media.src, '{}')", ItemModel().fieldsJsonObject(), app_cloud_name))
        .join(MediaModel())
        .filter(BaseModel<ItemModel>::Field::id.getFullFieldName(),
                Field::itemId.getFullFieldName(),
                false,
                std::string("="))
        .order_by(std::make_pair(MediaModel::Field::sort, true));

    QuerySet qs(BasketItemModel::tableName, limit, "data");
    qs.only(BasketItemModel().allSetFields())
        .join(ItemModel())
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
        .functions(Function(fmt::format(R"(, ({}) AS item)", qsItem.buildSelect())));
    applyFilters(qs, qsCount, params);
    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}

std::string BasketItemModel::fieldsJsonObject() {
    std::string app_cloud_name;
    getenv("APP_CLOUD_NAME", app_cloud_name);

    std::string str = BaseModel::fieldsJsonObject();
    QuerySet qs(ItemModel::tableName, "item", false, false);
    qs.jsonFields(fmt::format("{}, 'src', format_src(media.src, '{}')", ItemModel().fieldsJsonObject(), app_cloud_name))
        .filter(BaseModel<ItemModel>::Field::id.getFullFieldName(),
                Field::itemId.getFullFieldName(),
                false,
                std::string("="))
        .join(MediaModel())
        .order_by(std::make_pair(MediaModel::Field::sort, true));
    std::string sql = qs.buildSelect();
    str += fmt::format(", 'item', ({})", sql);
    return str;
}
