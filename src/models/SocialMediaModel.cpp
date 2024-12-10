#include "SocialMediaModel.h"
#include "ItemModel.h"

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> SocialMediaModel::joinMap() const {
    return {
        {ItemModel::tableName, {Field::itemId.getFullFieldName(), BaseModel<ItemModel>::Field::id.getFullFieldName()}}};
}

BaseModel<SocialMediaModel>::SetMapFieldTypes SocialMediaModel::getObjectValues() const {
    return {{std::cref(Field::title), title},
            {std::cref(Field::externalId), externalId},
            {std::cref(Field::itemId), itemId}};
}

std::string SocialMediaModel::fieldsJsonObject() {
    std::string ss = BaseModel().fieldsJsonObject();
    ss += fmt::format(", 'social_url', format_social_url({}, {}::TEXT)",
                      Field::externalId.getFullFieldName(),
                      Field::title.getFullFieldName());
    return ss;
}

std::string SocialMediaModel::sqlSelectList(const int page,
                                            int limit,
                                            const std::map<std::string, std::string, std::less<>> &params) {
    QuerySet qsCount = SocialMediaModel::qsCount();
    QuerySet qsPage = SocialMediaModel::qsPage(page, limit);
    QuerySet qs(tableName, limit, "data");
    qs.join(SocialMediaModel())
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
        .only(allSetFields())
        .order_by(std::make_pair(std::cref(BaseModel::Field::updatedAt), false))
        .functions(Function(fmt::format("format_social_url({}, {}::TEXT) as social_url",
                                        Field::externalId.getFullFieldName(),
                                        Field::title.getFullFieldName())));
    applyFilters(qs, qsCount, params);
    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}
