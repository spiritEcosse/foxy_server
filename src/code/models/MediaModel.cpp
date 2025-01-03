#include "MediaModel.h"
#include "ItemModel.h"
#include "env.h"
#include "fmt/format.h"

using namespace api::v1;

std::map<std::string, std::pair<std::string, std::string>, std::less<>> MediaModel::joinMap() const {
    return {
        {ItemModel::tableName, {Field::itemId.getFullFieldName(), BaseModel<ItemModel>::Field::id.getFullFieldName()}},
    };
}

BaseModel<MediaModel>::SetMapFieldTypes MediaModel::getObjectValues() const {
    return {{std::cref(Field::src), std::cref(src)},
            {std::cref(Field::itemId), std::cref(itemId)},
            {std::cref(Field::sort), std::cref(sort)},
            {std::cref(Field::contentType), std::cref(contentType)},
            {std::cref(Field::type), std::cref(type)}};
}

std::string MediaModel::fieldsJsonObject() {
    const Field field;
    std::vector<std::string> formattedFields;
    std::ranges::transform(field.allFields, std::back_inserter(formattedFields), [&](const auto& pair) {
        const auto& [key, value] = pair;
        if(key == "src") {
            return fmt::format("'{}', format_src({}, '{}')", key, value.get().getFullFieldName(), APP_CLOUD_NAME);
        }
        return fmt::format("'{}', {}", key, value.get().getFullFieldName());
    });

    return fmt::to_string(fmt::join(formattedFields, ", "));
}