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

        std::string title;
        std::string description;
        std::string slug;
        bool enabled = false;
        std::string metaDescription;
        ItemModel() = default;
        ItemModel(const ItemModel&) = delete;  // Copy constructor
        ItemModel& operator=(const ItemModel&) = delete;  // Copy assignment operator
        ItemModel(ItemModel&&) noexcept = default;  // Move constructor
        ItemModel& operator=(ItemModel&&) noexcept = default;  // Move assignment operator

        explicit ItemModel(const Json::Value& json) : BaseModel(json) {
            title = json[Field::title].asString();
            description = json[Field::description].asString();
            metaDescription = json[Field::metaDescription].asString();
            slug = json[Field::slug].asString();
            enabled = json[Field::enabled].asBool();

            Json::Value missingFields;
            validateField(Field::title, title, missingFields);
            validateField(Field::description, description, missingFields);
            validateField(Field::metaDescription, metaDescription, missingFields);
            validateField(Field::slug, slug, missingFields);
            checkMissingFields(missingFields);
        }

        [[nodiscard]] static std::vector<std::string> fields();
        [[nodiscard]] static std::vector<std::string> fullFields();
        [[nodiscard]] std::vector<
            std::pair<std::string, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
        getObjectValues() const override;
        [[nodiscard]] static std::string sqlSelectList(int page, int limit);
        [[nodiscard]] static std::string sqlSelectOne(const std::string& field, const std::string& value);
    };
}

#endif  //ITEMMODEL_H
