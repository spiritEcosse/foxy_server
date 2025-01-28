#include "SocialMediaModel.h"
#include "ItemModel.h"

using namespace api::v1;

BaseModelImpl::JoinMap SocialMediaModel::joinMap() const {
    return {{ItemModel::tableName, {&Field::itemId, &BaseModel<ItemModel>::Field::id}}};
}

BaseModel<SocialMediaModel>::SetMapFieldTypes SocialMediaModel::getObjectValues() const {
    return {{&Field::title, title}, {&Field::externalId, externalId}, {&Field::itemId, itemId}};
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
        .order_by(&BaseModel::Field::updatedAt, false)
        .functions(Function(fmt::format("format_social_url({}, {}::TEXT) as social_url",
                                        Field::externalId.getFullFieldName(),
                                        Field::title.getFullFieldName())));
    applyFilters(qs, qsCount, params);
    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}
