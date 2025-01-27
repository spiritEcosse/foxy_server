#pragma once

#include "BaseModel.h"

namespace api::v1 {
    class SocialMediaModel final : public BaseModel<SocialMediaModel> {
    public:
        using BaseModel::BaseModel;

        static const inline std::string tableName = "social_media";

        struct Field : BaseModel::Field {
            static inline const auto title = BaseField("title", tableName);
            static inline const auto externalId = BaseField("external_id", tableName);
            static inline const auto itemId = BaseField("item_id", tableName);

            Field() : BaseModel::Field() {
                constexpr std::array fields{&title, &externalId, &itemId};
                registerFields(fields);
            }
        };

        std::string title;
        std::string externalId;
        int itemId{};

        explicit SocialMediaModel(std::string title, std::string externalId, const int itemId) :
            title(std::move(title)), externalId(std::move(externalId)), itemId(itemId) {}

        explicit SocialMediaModel(const Json::Value &json) : BaseModel(json) {
            title = json[Field::title.getFieldName()].asString();
            externalId = json[Field::externalId.getFieldName()].asInt();
            itemId = json[Field::itemId.getFieldName()].asInt();

            validateField(Field::title.getFieldName(), title, missingFields);
            validateField(Field::externalId.getFieldName(), externalId, missingFields);
            validateField(Field::itemId.getFieldName(), itemId, missingFields);
        }

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
        [[nodiscard]] JoinMap joinMap() const override;
        [[nodiscard]] std::string fieldsJsonObject() override;
        [[nodiscard]] static std::string
        sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params);
    };

}
