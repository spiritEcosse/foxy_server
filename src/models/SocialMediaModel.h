//
// Created by ihor on 08.07.2024.
//

#ifndef SOCIALMEDIAMODEL_H
#define SOCIALMEDIAMODEL_H

#include "BaseModel.h"

namespace api::v1 {
    class SocialMediaModel : public BaseModel<SocialMediaModel> {
    public:
        static inline const std::string tableName = "social_media";

        struct Field : public BaseModel::Field {
            static inline BaseField title = BaseField("title", tableName);
            static inline BaseField externalId = BaseField("external_id", tableName);
            static inline BaseField itemId = BaseField("item_id", tableName);

            Field() : BaseModel<SocialMediaModel>::Field() {
                allFields[title.getFieldName()] = title;
                allFields[externalId.getFieldName()] = externalId;
                allFields[itemId.getFieldName()] = itemId;
            }
        };

        std::string title;
        std::string externalId;
        int itemId{};
        SocialMediaModel() = default;
        SocialMediaModel(const SocialMediaModel &) = delete;  // Copy constructor
        SocialMediaModel &operator=(const SocialMediaModel &) = delete;  // Copy assignment operator
        SocialMediaModel(SocialMediaModel &&) noexcept = default;  // Move constructor
        SocialMediaModel &operator=(SocialMediaModel &&) noexcept = default;  // Move assignment operator

        explicit SocialMediaModel(std::string title, std::string externalId, int itemId) :
            title(std::move(title)), externalId(std::move(externalId)), itemId(itemId) {}

        explicit SocialMediaModel(const Json::Value &json) : BaseModel(json) {
            title = json[Field::title.getFieldName()].asString();
            externalId = json[Field::externalId.getFieldName()].asInt();
            itemId = json[Field::itemId.getFieldName()].asInt();

            validateField(Field::title.getFieldName(), title, missingFields);
            validateField(Field::externalId.getFieldName(), externalId, missingFields);
            validateField(Field::itemId.getFieldName(), itemId, missingFields);
        }

        [[nodiscard]] static std::vector<BaseField> fields();
        [[nodiscard]] std::vector<
            std::pair<BaseField,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
        getObjectValues() const;
        [[nodiscard]] std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap() const;
    };

}

#endif  //SOCIALMEDIAMODEL_H
