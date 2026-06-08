#include "models/CollectionModel.h"
#include "models/ItemModel.h"
#include "models/CollectionItemModel.h"
#include "orm/QuerySet.h"
#include "utils/db/StringUtils.h"
#include <fmt/core.h>

using namespace api::v1;

BaseModelImpl::JoinMap CollectionModel::joinMap() {
    return {{CollectionItemModel::tableName, {&BaseModel::Field::id, &CollectionItemModel::Field::collectionId}}};
}

BaseModel<CollectionModel>::SetMapFieldTypes CollectionModel::getObjectValues() const {
    return {{&Field::title, title},
            {&Field::slug, slug},
            {&Field::description, description},
            {&Field::metaDescription, metaDescription},
            {&Field::enabled, enabled}};
}

QuerySet<CollectionModel> CollectionModel::qsCount() {
    QuerySet<CollectionModel> qsCount("total", false, true);
    return std::move(qsCount.filter(&Field::enabled, true).functions(Function("count(*)::integer")));
}

std::string
CollectionModel::sqlSelectList(const int page,
                               int limit,
                               [[maybe_unused]] const std::map<std::string, std::string, std::less<>> &params) {
    const auto &orderByCollection = &BaseModel::Field::updatedAt;
    auto qsPage = CollectionModel::qsPage(page, limit);

    QuerySet<CollectionModel> qs(limit, "data");
    qs.filter(&Field::enabled, true)
        .order_by(orderByCollection, false)
        .order_by(&BaseModel::Field::id, false)
        .only(allSetFields())
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit));
    return BuildComplexQueries::buildQuery(qsCount(), std::move(qsPage), std::move(qs));
}

std::string
CollectionModel::sqlSelectOne(const BaseField *field,
                              std::string &&value,
                              [[maybe_unused]] const std::map<std::string, std::string, std::less<>> &params) {
    QuerySet<ItemModel> qsItems(0, ItemModel::tableName, false);
    qsItems.join<CollectionItemModel>()
        .filter(&CollectionItemModel::Field::collectionId, &BaseModel::Field::id)
        .functions(Function(
            fmt::format("json_agg(json_build_object({}) ORDER BY item.id ASC)", ItemModel().fieldsJsonObject())));

    QuerySet<CollectionModel> qsCollection(tableName, true, true);
    qsCollection.filter(field, std::move(value))
        .jsonFields(addExtraQuotes(fieldsJsonObject()))
        .functions(
            Function(addExtraQuotes(fmt::format(R"( 'items', COALESCE(({}), '[]'::json))", qsItems.buildSelect()))));

    return qsCollection.buildSelectOne();
}
