#include "models/MediaModel.h"
#include "models/ItemModel.h"
#include "utils/config.h"
#include "fmt/format.h"

using namespace api::v1;

BaseModelImpl::JoinMap MediaModel::joinMap() {
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

QuerySet<MediaModel> MediaModel::qsMediaMinSort() {
    QuerySet<MediaModel> qs(0, tableName, false);
    qs.functions(Function(std::format("ROW_NUMBER() OVER (PARTITION BY {}, {} ORDER BY {} ASC) AS row_num",
                                      Field::itemId.getFullFieldName(),
                                      Field::type.getFullFieldName(),
                                      Field::sort.getFullFieldName())))
        .only(allSetFields());
    return qs;
}

std::string MediaModel::fieldsJsonObject() {
    const Field field;
    std::vector<std::string> formattedFields;
    std::ranges::transform(field.allFields, std::back_inserter(formattedFields), [&](const auto &pair) {
        const auto &[key, value] = pair;
        if(key == "src") {
            return fmt::format("'{}', format_src({}, '{}')", key, value->getFullFieldName(), getEnv("APP_CLOUD_NAME"));
        }
        return fmt::format("'{}', {}", key, value->getFullFieldName());
    });

    return fmt::to_string(fmt::join(formattedFields, ", "));
}

std::string
MediaModel::sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params) {
    auto qsCount = MediaModel::qsCount();
    auto qsPage = MediaModel::qsPage(page, limit);

    Field field;
    const auto orderIt = params.find("order");
    const auto orderValue = orderIt != params.end() ? orderIt->second : "";
    const auto it = field.allFields.find(orderValue);
    const auto &orderField = it != field.allFields.end() ? field.allFields.at(orderValue) : &Field::sort;

    const auto directionIt = params.find("direction");
    bool isAsc = true;
    if(directionIt != params.end()) {
        isAsc = directionIt->second == "asc";
    }

    QuerySet<MediaModel> qs(limit, "data");
    qs.offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
        .only(allSetFields())
        .functions(Function(fmt::format("format_src(media.src, '{}') as src", getEnv("APP_CLOUD_NAME"))))
        .order_by(orderField, isAsc)
        .order_by(&Field::id, false);
    applyFilters(qs, qsCount, params);
    return BuildComplexQueries::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}
