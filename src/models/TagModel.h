//
// Created by ihor on 28.09.2024.
//

#ifndef TAGMODEL_H
#define TAGMODEL_H

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"

namespace api::v1 {
    class TagModel : public BaseModel<TagModel> {
    public:
        static inline const std::string tableName = "tag";

        struct Field : public BaseModel::Field {
            static inline BaseField title = BaseField("title", tableName);
            static inline BaseField socialMedia = BaseField("social_media", tableName);
            static inline BaseField itemId = BaseField("item_id", tableName);

            Field() : BaseModel<TagModel>::Field() {
                allFields[title.getFieldName()] = title;
                allFields[socialMedia.getFieldName()] = socialMedia;
                allFields[itemId.getFieldName()] = itemId;
            }
        };

        std::string title;
        int itemId{};
        std::vector<std::string> socialMedia = {};

        bool enabled = false;
        TagModel() = default;
        TagModel(const TagModel &) = delete;  // Copy constructor
        TagModel &operator=(const TagModel &) = delete;  // Copy assignment operator
        TagModel(TagModel &&) noexcept = default;  // Move constructor
        TagModel &operator=(TagModel &&) noexcept = default;  // Move assignment operator

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

        [[nodiscard]] std::vector<
            std::pair<BaseField, std::variant<int, bool, std::vector<std::string>, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
    };
}

#endif  //TAGMODEL_H
