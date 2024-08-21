//
// Created by ihor on 14.01.2024.
//

#include "PageModel.h"

using namespace api::v1;

std::vector<std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
PageModel::getObjectValues() const {
    return {
        {Field::title, title},
        {Field::description, description},
        {Field::metaDescription, metaDescription},
        {Field::canonicalUrl, canonicalUrl},
        {Field::slug, slug},
        {Field::enabled, enabled},
    };
}
