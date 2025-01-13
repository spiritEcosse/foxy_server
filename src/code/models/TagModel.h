#pragma once

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"

namespace api::v1 {
    class TagModel final : public BaseModel<TagModel> {
    public:
        using BaseModel::BaseModel;

        static const inline std::string tableName = "tag";

        struct Field : BaseModel::Field {
            static inline const auto title = BaseField("title", tableName);
            static inline const auto socialMedia = BaseField("social_media", tableName);
            static inline const auto itemId = BaseField("item_id", tableName);

            Field() : BaseModel::Field() {
                allFields.try_emplace(title.getFieldName(), std::cref(title));
                allFields.try_emplace(socialMedia.getFieldName(), std::cref(socialMedia));
                allFields.try_emplace(itemId.getFieldName(), std::cref(itemId));
            }
        };

        std::string title;
        int itemId{};
        std::vector<std::string> socialMedia = {};

        bool enabled = false;

        explicit TagModel(const Json::Value &json) : BaseModel(json) {
            title = json[Field::title.getFieldName()].asString();
            itemId = json[Field::itemId.getFieldName()].asInt();
            for(const auto &tag: json[Field::socialMedia.getFieldName()]) {
                socialMedia.push_back(tag.asString());
            }

            validateField(Field::title.getFieldName(), title, missingFields);
            validateField(Field::itemId.getFieldName(), itemId, missingFields);
            validateField(Field::socialMedia.getFieldName(), socialMedia, missingFields);
        }

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
        [[nodiscard]] std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap() const override;
    };
}
