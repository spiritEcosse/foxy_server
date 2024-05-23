//
// Created by ihor on 14.01.2024.
//

#ifndef PAGEMODEL_H
#define PAGEMODEL_H

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"

namespace api::v1 {
    class PageModel : public BaseModel<PageModel> {
    public:
        static inline const std::string tableName = "page";

        struct Field : public BaseModel::Field {
            static inline BaseField<PageModel> title = BaseField<PageModel>("title");
            static inline BaseField<PageModel> description = BaseField<PageModel>("description");
            static inline BaseField<PageModel> metaDescription = BaseField<PageModel>("meta_description");
            static inline BaseField<PageModel> slug = BaseField<PageModel>("slug");
            static inline BaseField<PageModel> canonicalUrl = BaseField<PageModel>("canonical_url");
            static inline BaseField<PageModel> enabled = BaseField<PageModel>("enabled");
        };

        std::string title;
        std::string slug;
        std::string description;
        std::string metaDescription;
        std::string canonicalUrl;
        bool enabled = false;
        PageModel() = default;
        PageModel(const PageModel &) = delete;  // Copy constructor
        PageModel &operator=(const PageModel &) = delete;  // Copy assignment operator
        PageModel(PageModel &&) noexcept = default;  // Move constructor
        PageModel &operator=(PageModel &&) noexcept = default;  // Move assignment operator

        explicit PageModel(const Json::Value &json) : BaseModel(json) {
            title = json[Field::title.getFieldName()].asString();
            description = json[Field::description.getFieldName()].asString();
            metaDescription = json[Field::metaDescription.getFieldName()].asString();
            canonicalUrl = json[Field::canonicalUrl.getFieldName()].asString();
            slug = json[Field::slug.getFieldName()].asString();
            enabled = json[Field::enabled.getFieldName()].asBool();

            Json::Value missingFields;
            validateField(Field::title.getFieldName(), title, missingFields);
            validateField(Field::description.getFieldName(), description, missingFields);
            validateField(Field::metaDescription.getFieldName(), metaDescription, missingFields);
            validateField(Field::canonicalUrl.getFieldName(), canonicalUrl, missingFields);
            validateField(Field::slug.getFieldName(), slug, missingFields);
        }

        [[nodiscard]] static std::vector<BaseField<PageModel>> fields();
        [[nodiscard]] static std::vector<BaseField<PageModel>> fullFields();
        [[nodiscard]] std::vector<
            std::pair<BaseField<PageModel>,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const;
    };
}

#endif  //PAGEMODEL_H
