#include "MediaModel.h"
#include "ItemModel.h"
#include "env.h"
#include "fmt/format.h"

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> MediaModel::joinMap() const {
    return {
        {ItemModel::tableName, {Field::itemId.getFullFieldName(), BaseModel<ItemModel>::Field::id.getFullFieldName()}},
    };
}

BaseModel<MediaModel>::SetMapFieldTypes MediaModel::getObjectValues() const {
    return {{std::cref(Field::src), std::cref(src)},
            {std::cref(Field::itemId), std::cref(itemId)},
            {std::cref(Field::sort), std::cref(sort)},
            {std::cref(Field::contentType), std::cref(contentType)},
            {std::cref(Field::type), std::cref(type)}};
}

std::string MediaModel::fieldsJsonObject() {
    const Field field;
    std::vector<std::string> formattedFields;
    std::ranges::transform(field.allFields, std::back_inserter(formattedFields), [&](const auto &pair) {
        const auto &[key, value] = pair;
        if(key == "src") {
            return fmt::format("'{}', format_src({}, '{}')", key, value.get().getFullFieldName(), APP_CLOUD_NAME);
        }
        return fmt::format("'{}', {}", key, value.get().getFullFieldName());
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
    const auto &orderField =
        (it != field.allFields.end()) ? std::cref(field.allFields.at(orderValue)) : std::cref(field.updatedAt);

    const auto directionIt = params.find("direction");
    bool isAsc = directionIt != params.end() && directionIt->second == "asc";

    QuerySet qs(tableName, limit, "data");
    qs.offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
        .only(allSetFields())
        .functions(Function(fmt::format("format_src(media.src, '{}') as src", APP_CLOUD_NAME)))
        .order_by(std::make_pair(orderField, isAsc), std::make_pair(std::cref(Field::id), false));
    applyFilters(qs, qsCount, params);
    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}
