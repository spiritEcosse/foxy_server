#pragma once

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"
#include "decimal.h"

namespace api::v1 {

    class ItemModel final : public BaseModel<ItemModel> {
    public:
        using BaseModel::BaseModel;
        static const inline std::string tableName = "item";

        struct Field : BaseModel::Field {
            static inline const auto title = BaseField("title", tableName);
            static inline const auto description = BaseField("description", tableName);
            static inline const auto metaDescription = BaseField("meta_description", tableName);
            static inline const auto shippingProfileId = BaseField("shipping_profile_id", tableName);
            static inline const auto slug = BaseField("slug", tableName);
            static inline const auto enabled = BaseField("enabled", tableName);
            static inline const auto price = BaseField("price", tableName);

            Field() : BaseModel::Field() {
                allFields.try_emplace(title.getFieldName(), std::cref(title));
                allFields.try_emplace(description.getFieldName(), std::cref(description));
                allFields.try_emplace(metaDescription.getFieldName(), std::cref(metaDescription));
                allFields.try_emplace(shippingProfileId.getFieldName(), std::cref(shippingProfileId));
                allFields.try_emplace(slug.getFieldName(), std::cref(slug));
                allFields.try_emplace(price.getFieldName(), std::cref(price));
                allFields.try_emplace(enabled.getFieldName(), std::cref(enabled));
            }
        };

        // start fields
        std::string title;
        int shippingProfileId{};
        std::string description;
        std::string slug;
        bool enabled = false;
        dec::decimal<2> price;
        std::string metaDescription;

        // end fields

        explicit ItemModel(const Json::Value &json) : BaseModel(json) {
            title = json[Field::title.getFieldName()].asString();
            description = json[Field::description.getFieldName()].asString();
            metaDescription = json[Field::metaDescription.getFieldName()].asString();
            slug = json[Field::slug.getFieldName()].asString();
            shippingProfileId = json[Field::shippingProfileId.getFieldName()].asInt();
            enabled = json[Field::enabled.getFieldName()].asBool();
            price = json[Field::price.getFieldName()].asDouble();

            validateField(Field::title.getFieldName(), title, missingFields);
            validateField(Field::description.getFieldName(), description, missingFields);
            validateField(Field::shippingProfileId.getFieldName(), shippingProfileId, missingFields);
            validateField(Field::metaDescription.getFieldName(), metaDescription, missingFields);
            validateField(Field::slug.getFieldName(), slug, missingFields);
            validateField(Field::price.getFieldName(), price, missingFields);
        }

        [[nodiscard]] static QuerySet qsCount();

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
        [[nodiscard]] static std::string
        sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params);
        [[nodiscard]] std::string sqlSelectOne(const std::string &field,
                                               const std::string &value,
                                               const std::map<std::string, std::string, std::less<>> &params) override;
        [[nodiscard]] std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap() const override;
    };
}
