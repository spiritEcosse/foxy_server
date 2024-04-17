//
// Created by ihor on 20.01.2024.
//

#include "MediaModel.h"
#include <fmt/core.h>
#include "src/utils/env.h"

using namespace api::v1;

std::vector<std::string> MediaModel::fields() {
    return {
        Field::updatedAt,
        Field::src,
        Field::itemId,
        Field::sort,
    };
}

std::vector<std::string> MediaModel::fullFields() {
    return {
        Field::id,
        Field::itemId,
        Field::sort,
        Field::src,
        Field::createdAt,
        Field::updatedAt,
    };
}

std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
MediaModel::getObjectValues() const {
    auto baseValues = BaseModel::getObjectValues();
    baseValues.emplace_back(Field::src, src);
    baseValues.emplace_back(Field::itemId, itemId);
    baseValues.emplace_back(Field::sort, sort);
    return baseValues;
}

std::string MediaModel::fieldsJsonObject() {
    std::string app_cloud_name;
    getenv("APP_CLOUD_NAME", app_cloud_name);

    std::stringstream ss;
    for(auto fieldNames = fullFields(); const auto &fieldName: fieldNames) {
        if(fieldName == "src") {
            ss << fmt::format("'{}', format_src({}, '{}') ", fieldName, fieldName, app_cloud_name);
        } else {
            ss << fmt::format("'{}', {} ", fieldName, fieldName);
        }
        if(&fieldName != &fieldNames.back()) {
            ss << ", ";
        }
    }
    return ss.str();
}
