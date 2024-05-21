//
// Created by ihor on 20.01.2024.
//

#ifndef MEDIAMODEL_H
#define MEDIAMODEL_H

#include "BaseModel.h"
#include "json/json.h"
#include "ItemModel.h"

namespace api::v1 {
    class MediaModel : public BaseModel<MediaModel> {
    public:
        struct Field : public BaseModel::Field {
            static inline const std::string src = "src";
            static inline const std::string itemId = "item_id";
            static inline const std::string sort = "sort";
        };

        static inline std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap = {
            {ItemModel::tableName, {Field::itemId, "id"}}, // Check it, why hard code "id", ItemModel::Field::id gives empty string
        };

        static inline const std::string tableName = "media";
        MediaModel() = default;
        MediaModel(const MediaModel&) = delete;  // Copy constructor
        MediaModel& operator=(const MediaModel&) = delete;  // Copy assignment operator
        MediaModel(MediaModel&&) noexcept = default;  // Move constructor
        MediaModel& operator=(MediaModel&&) noexcept = default;  // Move assignment operator
        std::string src;
        int itemId = 0;
        int sort = 0;

        explicit MediaModel(const Json::Value& json) : BaseModel(json) {
            Json::Value missingFields;

            src = json[Field::src].asString();
            if(src.empty()) {
                missingFields[Field::src] = Field::src + " is required";
            }
            id = json[Field::id].asInt();
            itemId = json[Field::itemId].asInt();
            if(!itemId) {
                missingFields[Field::itemId] = Field::itemId + " is required";
            }
            sort = json[Field::sort].asInt();
            if(!sort) {
                missingFields[Field::sort] = Field::sort + " is required";
            }
        }

        [[nodiscard]] static std::string fieldsJsonObject();
        [[nodiscard]] static std::vector<std::string> fields();
        [[nodiscard]] static std::vector<std::string> fullFields();
        [[nodiscard]] std::vector<
            std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
    };
}

#endif  //MEDIAMODEL_H
