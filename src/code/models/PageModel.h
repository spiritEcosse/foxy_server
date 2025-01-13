#pragma once

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"

namespace api::v1 {
    class PageModel final : public BaseModel<PageModel> {
    public:
        using BaseModel::BaseModel;

        static const inline std::string tableName = "page";

        struct Field : BaseModel::Field {
            static inline const auto title = BaseField("title", tableName);
            static inline const auto description = BaseField("description", tableName);
            static inline const auto metaDescription = BaseField("meta_description", tableName);
            static inline const auto slug = BaseField("slug", tableName);
            static inline const auto canonicalUrl = BaseField("canonical_url", tableName);
            static inline const auto enabled = BaseField("enabled", tableName);

            Field() : BaseModel::Field() {
                allFields.try_emplace(title.getFieldName(), std::cref(title));
                allFields.try_emplace(description.getFieldName(), std::cref(description));
                allFields.try_emplace(metaDescription.getFieldName(), std::cref(metaDescription));
                allFields.try_emplace(slug.getFieldName(), std::cref(slug));
                allFields.try_emplace(canonicalUrl.getFieldName(), std::cref(canonicalUrl));
                allFields.try_emplace(enabled.getFieldName(), std::cref(enabled));
            }
        };

        std::string title;
        std::string slug;
        std::string description;
        std::string metaDescription;
        std::string canonicalUrl;
        bool enabled = false;

        explicit PageModel(const Json::Value &json) : BaseModel(json) {
            title = json[Field::title.getFieldName()].asString();
            description = json[Field::description.getFieldName()].asString();
            metaDescription = json[Field::metaDescription.getFieldName()].asString();
            canonicalUrl = json[Field::canonicalUrl.getFieldName()].asString();
            slug = json[Field::slug.getFieldName()].asString();
            enabled = json[Field::enabled.getFieldName()].asBool();

            validateField(Field::title.getFieldName(), title, missingFields);
            validateField(Field::description.getFieldName(), description, missingFields);
            validateField(Field::metaDescription.getFieldName(), metaDescription, missingFields);
            validateField(Field::canonicalUrl.getFieldName(), canonicalUrl, missingFields);
            validateField(Field::slug.getFieldName(), slug, missingFields);
        }

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
    };
}
