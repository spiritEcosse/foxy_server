//
// Created by ihor on 14.01.2024.
//

#ifndef PAGEMODEL_H
#define PAGEMODEL_H

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"
#include "src/utils/exceptions/RequiredFieldsException.h"

namespace api::v1 {
    class PageModel : public BaseModel<PageModel> {
    public:
        struct Field : public BaseModel::Field {
            static inline const std::string title = "title";
            static inline const std::string description = "description";
            static inline const std::string metaDescription = "meta_description";
            static inline const std::string slug = "slug";
            static inline const std::string canonicalUrl = "canonical_url";
        };

        static inline const std::string tableName = "page";

        int id = 0;
        std::string title;
        std::string slug;
        std::string description;
        std::string metaDescription;
        std::string canonicalUrl;
        std::chrono::system_clock::time_point createdAt;
        std::chrono::system_clock::time_point updatedAt;
        PageModel() = default;
        PageModel(const PageModel&) = delete;  // Copy constructor
        PageModel& operator=(const PageModel&) = delete;  // Copy assignment operator
        PageModel(PageModel&&) noexcept = default;  // Move constructor
        PageModel& operator=(PageModel&&) noexcept = default;  // Move assignment operator

        explicit PageModel(const Json::Value& json) {
            title = json[Field::title].asString();
            description = json[Field::description].asString();
            metaDescription = json[Field::metaDescription].asString();
            canonicalUrl = json[Field::canonicalUrl].asString();
            slug = json[Field::slug].asString();

            Json::Value missingFields;
            validateField(Field::title, title, missingFields);
            validateField(Field::description, description, missingFields);
            validateField(Field::metaDescription, metaDescription, missingFields);
            validateField(Field::canonicalUrl, canonicalUrl, missingFields);
            validateField(Field::slug, slug, missingFields);
            checkMissingFields(missingFields);
        }

        [[nodiscard]] static std::vector<std::string> fields();
        [[nodiscard]] static std::vector<std::string> fullFields();
        [[nodiscard]] std::vector<std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>> getObjectValues() const override;
    };
}

#endif  //PAGEMODEL_H
