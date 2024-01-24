//
// Created by ihor on 14.01.2024.
//

#include "PageModel.h"

using namespace api::v1;

std::vector<std::string> PageModel::fields() {
    return {
        Field::title,
        Field::description,
        Field::metaDescription,
        Field::canonicalUrl,
        Field::slug,
    };
}

std::vector<std::string> PageModel::fullFields() {
    return {
        Field::id,
        Field::title,
        Field::description,
        Field::metaDescription,
        Field::canonicalUrl,
        Field::slug,
        Field::createdAt,
        Field::updatedAt,
    };
}

std::vector<std::pair<std::string, std::variant<int, bool, std::string>>> PageModel::getObjectValues() const {
    return {
        {Field::title, title},
        {Field::description, description},
        {Field::metaDescription, metaDescription},
        {Field::canonicalUrl, canonicalUrl},
        {Field::slug, slug},
    };
}
