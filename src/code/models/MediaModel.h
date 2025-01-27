#pragma once

#include "BaseModel.h"
#include "json/json.h"

namespace api::v1 {
    struct MediaType final {
        static inline std::string VIDEO = "video";
        static inline std::string IMAGE = "image";
    };

    class MediaModel final : public BaseModel<MediaModel> {
    public:
        using BaseModel::BaseModel;

        static const inline std::string tableName = "media";

        struct Field : public BaseModel::Field {
            static inline const auto src = BaseField("src", tableName);
            static inline const auto itemId = BaseField("item_id", tableName);
            static inline const auto sort = BaseField("sort", tableName);
            static inline const auto type = BaseField("type", tableName);
            static inline const auto contentType = BaseField("content_type", tableName);

            Field() : BaseModel::Field() {
                constexpr std::array fields{&src, &itemId, &sort, &type, &contentType};
                registerFields(fields);
            }
        };

        std::string src;
        std::string type;
        std::string contentType;
        int itemId = 0;
        int sort = 0;

        explicit MediaModel(const Json::Value &json) : BaseModel(json) {
            src = json[Field::src.getFieldName()].asString();
            itemId = json[Field::itemId.getFieldName()].asInt();
            sort = json[Field::sort.getFieldName()].asInt();
            type = json[Field::type.getFieldName()].asString();
            contentType = json[Field::contentType.getFieldName()].asString();

            validateField(Field::src.getFieldName(), src, missingFields);
            validateField(Field::contentType.getFieldName(), contentType, missingFields);
            validateField(Field::itemId.getFieldName(), itemId, missingFields);
            validateField(Field::sort.getFieldName(), sort, missingFields);
            validateField(Field::type.getFieldName(), type, missingFields);
        }

        [[nodiscard]] std::string fieldsJsonObject() override;
        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
        [[nodiscard]] JoinMap joinMap() const override;
        [[nodiscard]] static std::string
        sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params);
    };
}
