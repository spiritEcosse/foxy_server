#include "MediaModel.h"
#include "ItemModel.h"
#include "env.h"
#include "fmt/format.h"

using namespace api::v1;

BaseModelImpl::JoinMap MediaModel::joinMap() const {
    return {
        {ItemModel::tableName, {&Field::itemId, &BaseModel<ItemModel>::Field::id}},
    };
}

BaseModel<MediaModel>::SetMapFieldTypes MediaModel::getObjectValues() const {
    return {{&Field::src, src},
            {&Field::itemId, itemId},
            {&Field::sort, sort},
            {&Field::contentType, contentType},
            {&Field::type, type}};
}

std::string MediaModel::fieldsJsonObject() {
    const Field field;
    std::vector<std::string> formattedFields;
    std::ranges::transform(field.allFields, std::back_inserter(formattedFields), [&](const auto &pair) {
        const auto &[key, value] = pair;
        if(key == "src") {
            return fmt::format("'{}', format_src({}, '{}')", key, value->getFullFieldName(), APP_CLOUD_NAME);
        }
        return fmt::format("'{}', {}", key, value->getFullFieldName());
    });

    return fmt::to_string(fmt::join(formattedFields, ", "));
}

std::string
MediaModel::sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params) {
    QuerySet qsCount = MediaModel::qsCount();
    QuerySet qsPage = MediaModel::qsPage(page, limit);

    Field field;
    const auto orderIt = params.find("order");
    const auto orderValue = orderIt != params.end() ? orderIt->second : "";
    const auto it = field.allFields.find(orderValue);
    const auto &orderField = (it != field.allFields.end()) ? field.allFields.at(orderValue) : &field.updatedAt;

    const auto directionIt = params.find("direction");
    bool isAsc = directionIt != params.end() && directionIt->second == "asc";

    QuerySet qs(tableName, limit, "data");
    qs.offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
        .only(allSetFields())
        .functions(Function(fmt::format("format_src(media.src, '{}') as src", APP_CLOUD_NAME)))
        .order_by(orderField, isAsc)
        .order_by(&Field::id, false);
    applyFilters(qs, qsCount, params);
    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}
