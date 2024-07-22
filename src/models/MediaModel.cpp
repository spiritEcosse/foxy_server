//
// Created by ihor on 20.01.2024.
//

#include "MediaModel.h"
#include "ItemModel.h"
#include <fmt/core.h>
#include "env.h"

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> MediaModel::joinMap() const {
    return {
        {ItemModel::tableName,
         {MediaModel::Field::itemId.getFullFieldName(), BaseModel<ItemModel>::Field::id.getFullFieldName()}},
    };
}

std::vector<std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
MediaModel::getObjectValues() const {
    return {
        {Field::src, src},
        {Field::itemId, itemId},
        {Field::sort, sort},
    };
}

std::string MediaModel::fieldsJsonObject() {
    std::string app_cloud_name;
    getenv("APP_CLOUD_NAME", app_cloud_name);

    std::stringstream ss;
    const Field field;
    for(const auto &fieldNames = field.allFields; const auto &pair: fieldNames) {
        if(pair.first == "src") {
            ss << fmt::format("'{}', format_src({}, '{}'),",
                              pair.first,
                              pair.second.getFullFieldName(),
                              app_cloud_name);
        } else {
            ss << fmt::format("'{}', {},", pair.first, pair.second.getFullFieldName());
        }
    }
    // remove last comma
    ss.seekp(-1, std::ios_base::end);
    ss << ' ';  // overwrite the comma with a space
    return ss.str();
}
