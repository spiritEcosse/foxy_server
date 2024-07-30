//
// Created by ihor on 08.07.2024.
//

#include "SocialMediaModel.h"
#include "ItemModel.h"

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> SocialMediaModel::joinMap() const {
    return {
        {ItemModel::tableName, {Field::itemId.getFullFieldName(), BaseModel<ItemModel>::Field::id.getFullFieldName()}}};
}

std::vector<
    std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
SocialMediaModel::getObjectValues() const {
    return {
        {Field::title, title},
        {Field::externalId, externalId},
        {Field::itemId, itemId},
    };
}

std::string SocialMediaModel::fieldsJsonObject() {
    std::string ss = BaseModel<SocialMediaModel>().fieldsJsonObject();
    ss += fmt::format(", 'social_url', format_social_url({}, {}::TEXT)",
                      SocialMediaModel::Field::externalId.getFullFieldName(),
                      SocialMediaModel::Field::title.getFullFieldName());
    return ss;
}

std::string
SocialMediaModel::sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params) {
    QuerySet qsCount = SocialMediaModel().qsCount();
    QuerySet qsPage = SocialMediaModel().qsPage(page, limit);
    QuerySet qs(tableName, limit, "data");
    qs.join(SocialMediaModel())
        .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
        .only(allSetFields())
        .order_by(std::make_pair(BaseModel<SocialMediaModel>::Field::updatedAt, false))
        .functions(Function(fmt::format("format_social_url({}, {}::TEXT) as social_url",
                                        SocialMediaModel::Field::externalId.getFullFieldName(),
                                        SocialMediaModel::Field::title.getFullFieldName())));
    applyFilters(qs, qsCount, params);
    return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
}
