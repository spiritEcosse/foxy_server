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

std::vector<BaseField> BasketItemModel::fields() {
    return {Field::basketId, Field::itemId, Field::quantity, Field::price};
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
    std::string jsonFieldsItem = ItemModel().fieldsJsonObject();
    jsonFieldsItem += fmt::format(", 'src', format_src(media.src, '{}')", app_cloud_name);
    qsItem.jsonFields(jsonFieldsItem)
        .join(MediaModel())
        .filter(ItemModel::Field::id.getFullFieldName(), Field::itemId.getFullFieldName(), false, std::string("="))
        .order_by(std::make_pair(MediaModel::Field::sort, true));

    QuerySet qs(BasketItemModel::tableName, limit, "data");
    qs.only(BasketItemModel().allSetFields())
        .join(ItemModel())
        .functions(Function(fmt::format(R"(, ({}) AS item)", qsItem.buildSelect())));
    applyFilters(qs, qsCount, params);
    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}

std::string BasketItemModel::fieldsJsonObject() {
    std::string app_cloud_name;
    getenv("APP_CLOUD_NAME", app_cloud_name);

    std::string str = BaseModel::fieldsJsonObject();
    QuerySet qs(ItemModel::tableName, "item", false, false);
    std::string jsonFields = ItemModel().fieldsJsonObject();
    jsonFields += fmt::format(", 'src', format_src(media.src, '{}')", app_cloud_name);
    qs.jsonFields(jsonFields)
        .filter(ItemModel::Field::id.getFullFieldName(), Field::itemId.getFullFieldName(), false, std::string("="))
        .join(MediaModel())
        .order_by(std::make_pair(MediaModel::Field::sort, true));
    std::string sql = qs.buildSelect();
    str += fmt::format(", 'item', ({})", sql);
    return str;
}
