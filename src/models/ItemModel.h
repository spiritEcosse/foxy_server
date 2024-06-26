//
// Created by ihor on 13.01.2024.
//

#ifndef ITEMMODEL_H
#define ITEMMODEL_H

#include <string>
#include <chrono>
#include <drogon/drogon.h>
#include "BaseModel.h"
#include <iomanip>
#include <iostream>
#include "decimal.h"

namespace api::v1 {

    class ItemModel : public BaseModel<ItemModel> {
    public:
        struct Field : public BaseModel::Field {
            static inline const std::string title = "title";
            static inline const std::string description = "description";
            static inline const std::string metaDescription = "meta_description";
            static inline const std::string shippingProfileId = "shipping_profile_id";
            static inline const std::string slug = "slug";
            static inline const std::string enabled = "enabled";
            static inline const std::string price = "price";
        };

        static inline const std::string tableName = "item";

        std::string title;
        int shippingProfileId{};
        std::string description;
        std::string slug;
        bool enabled = false;
        dec::decimal<2> price;
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
            shippingProfileId = json[Field::shippingProfileId].asInt();
            enabled = json[Field::enabled].asBool();
            auto priceString = json[Field::price].asString();

            validateField(Field::title, title, missingFields);
            validateField(Field::description, description, missingFields);
            validateField(Field::shippingProfileId, shippingProfileId, missingFields);
            validateField(Field::metaDescription, metaDescription, missingFields);
            validateField(Field::slug, slug, missingFields);
            validateField(Field::price, priceString, missingFields);
            if(missingFields.empty()) {
                price = std::stod(priceString);
            }
        }

        [[nodiscard]] static QuerySet qsCount();

        [[nodiscard]] static std::vector<std::string> fields();
        [[nodiscard]] static std::vector<std::string> fullFields();
        [[nodiscard]] std::vector<
            std::pair<std::string,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
        getObjectValues() const;
        [[nodiscard]] static std::string sqlSelectList(int page, int limit);
        [[nodiscard]] static std::string
        sqlSelectOne(const std::string& field,
                     const std::string& value,
                     const std::map<std::string, std::string, std::less<>>& params = {});
    };
}

#endif  //ITEMMODEL_H
