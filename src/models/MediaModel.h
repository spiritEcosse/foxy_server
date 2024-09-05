//
// Created by ihor on 20.01.2024.
//

#ifndef MEDIAMODEL_H
#define MEDIAMODEL_H

#include "BaseModel.h"
#include "json/json.h"

namespace api::v1 {
    class MediaModel : public BaseModel<MediaModel> {
    public:
        static inline const std::string tableName = "media";

        struct Field : public BaseModel::Field {
            static inline BaseField src = BaseField("src", tableName);
            static inline BaseField itemId = BaseField("item_id", tableName);
            static inline BaseField sort = BaseField("sort", tableName);
            static inline BaseField type = BaseField("type", tableName);

            Field() : BaseModel<MediaModel>::Field() {
                allFields[src.getFieldName()] = src;
                allFields[itemId.getFieldName()] = itemId;
                allFields[sort.getFieldName()] = sort;
                allFields[type.getFieldName()] = type;
            }
        };

        MediaModel() = default;
        MediaModel(const MediaModel &) = delete;  // Copy constructor
        MediaModel &operator=(const MediaModel &) = delete;  // Copy assignment operator
        MediaModel(MediaModel &&) noexcept = default;  // Move constructor
        MediaModel &operator=(MediaModel &&) noexcept = default;  // Move assignment operator
        std::string src;
        std::string type;
        int itemId = 0;
        int sort = 0;

        explicit MediaModel(const Json::Value &json) : BaseModel(json) {
            src = json[Field::src.getFieldName()].asString();
            itemId = json[Field::itemId.getFieldName()].asInt();
            sort = json[Field::sort.getFieldName()].asInt();
            type = json[Field::type.getFieldName()].asString();

            validateField(Field::src.getFieldName(), src, missingFields);
            validateField(Field::itemId.getFieldName(), itemId, missingFields);
            validateField(Field::sort.getFieldName(), sort, missingFields);
            validateField(Field::type.getFieldName(), type, missingFields);
        }

        [[nodiscard]] std::string fieldsJsonObject() override;
        [[nodiscard]] std::vector<
            std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
        [[nodiscard]] std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap() const override;
    };
}

#endif  //MEDIAMODEL_H
