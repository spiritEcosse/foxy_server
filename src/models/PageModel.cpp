//
// Created by ihor on 14.01.2024.
//

#include "PageModel.h"

using namespace api::v1;

std::vector<std::string> PageModel::fields() {
    return {
        Field::updatedAt,
        Field::title,
        Field::description,
        Field::metaDescription,
        Field::canonicalUrl,
        Field::slug,
        Field::enabled,
    };
}

std::vector<std::string> PageModel::fullFields() {
    return {
        Field::id,
        Field::title,
        Field::enabled,
        Field::description,
        Field::metaDescription,
        Field::canonicalUrl,
        Field::slug,
        Field::createdAt,
        Field::updatedAt,
    };
}

std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
PageModel::getObjectValues() const {
    auto baseValues = BaseModel::getObjectValues();
    baseValues.emplace_back(Field::title, title);
    baseValues.emplace_back(Field::description, description);
    baseValues.emplace_back(Field::metaDescription, metaDescription);
    baseValues.emplace_back(Field::canonicalUrl, canonicalUrl);
    baseValues.emplace_back(Field::slug, slug);
    baseValues.emplace_back(Field::enabled, enabled);
    return baseValues;
}
