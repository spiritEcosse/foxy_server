//
// Created by ihor on 20.01.2024.
//

#include "MediaModel.h"
#include "ItemModel.h"
#include "env.h"
#include "fmt/format.h"

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

    const Field field;
    std::vector<std::string> formattedFields;
    std::ranges::transform(field.allFields, std::back_inserter(formattedFields), [&](const auto& pair) {
        const auto& [key, value] = pair;
        if(key == "src") {
            return fmt::format("'{}', format_src({}, '{}')", key, value.getFullFieldName(), app_cloud_name);
        } else {
            return fmt::format("'{}', {}", key, value.getFullFieldName());
        }
    });

    return fmt::to_string(fmt::join(formattedFields, ", "));
}
