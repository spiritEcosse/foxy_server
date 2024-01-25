//
// Created by ihor on 13.01.2024.
//

#ifndef ITEMMODEL_H
#define ITEMMODEL_H

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"
#include "src/utils/exceptions/RequiredFieldsException.h"

namespace api::v1 {

    class ItemModel : public BaseModel<ItemModel> {
    public:
        struct Field : public BaseModel::Field {
            static inline const std::string title = "title";
            static inline const std::string description = "description";
            static inline const std::string metaDescription = "meta_description";
            static inline const std::string slug = "slug";
            static inline const std::string enabled = "enabled";
        };

        static inline const std::string tableName = "item";

        int id = 0;
        std::string title;
        std::string description;
        std::string slug;
        bool enabled = false;
        std::string metaDescription;
        std::chrono::system_clock::time_point createdAt;
        std::chrono::system_clock::time_point updatedAt;
        ItemModel() = default;
        ItemModel(const ItemModel&) = delete;  // Copy constructor
        ItemModel& operator=(const ItemModel&) = delete;  // Copy assignment operator
        ItemModel(ItemModel&&) noexcept = default;  // Move constructor
        ItemModel& operator=(ItemModel&&) noexcept = default;  // Move assignment operator

        explicit ItemModel(const Json::Value& json) {
            title = json[Field::title].asString();
            description = json[Field::description].asString();
            metaDescription = json[Field::metaDescription].asString();
            slug = json[Field::slug].asString();
            enabled = json[Field::enabled].asBool();

            Json::Value missingFields;
            if(title.empty()) {
                missingFields[Field::title] = Field::title + " is required";
            }
            if(description.empty()) {
                missingFields[Field::description] = Field::description + " is required";
            }
            if(metaDescription.empty()) {
                missingFields[Field::metaDescription] = Field::metaDescription + " is required";
            }
            if(slug.empty()) {
                missingFields[Field::slug] = Field::slug + " is required";
            }

            if(!missingFields.empty()) {
                throw RequiredFieldsException(missingFields);
            }
        }

        [[nodiscard]] static std::vector<std::string> fields();
        [[nodiscard]] static std::vector<std::string> fullFields();
        [[nodiscard]] std::vector<std::pair<std::string, std::variant<int, bool, std::string>>> getObjectValues() const;
        [[nodiscard]] static std::string
        sqlSelectList(int page, int limit, const std::unordered_map<std::string, std::string>& params);
        [[nodiscard]] static std::string sqlSelectOne(const std::string& field, const std::string& value);
    };
}

#endif  //ITEMMODEL_H
