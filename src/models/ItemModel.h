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
        static inline const std::string tableName = "item";

        struct Field : public BaseModel::Field {
            static inline BaseField title = BaseField("title", tableName);
            static inline BaseField description = BaseField("description", tableName);
            static inline BaseField metaDescription = BaseField("meta_description", tableName);
            static inline BaseField shippingProfileId = BaseField("shipping_profile_id", tableName);
            static inline BaseField slug = BaseField("slug", tableName);
            static inline BaseField enabled = BaseField("enabled", tableName);
            static inline BaseField price = BaseField("price", tableName);

            Field() : BaseModel<ItemModel>::Field() {
                allFields[title.getFieldName()] = title;
                allFields[description.getFieldName()] = description;
                allFields[metaDescription.getFieldName()] = metaDescription;
                allFields[shippingProfileId.getFieldName()] = shippingProfileId;
                allFields[slug.getFieldName()] = slug;
                allFields[enabled.getFieldName()] = enabled;
                allFields[price.getFieldName()] = price;
            }
        };

        std::string title;
        int shippingProfileId{};
        std::string description;
        std::string slug;
        bool enabled = false;
        dec::decimal<2> price;
        std::string metaDescription;
        ItemModel() = default;
        ItemModel(const ItemModel &) = delete;  // Copy constructor
        ItemModel &operator=(const ItemModel &) = delete;  // Copy assignment operator
        ItemModel(ItemModel &&) noexcept = default;  // Move constructor
        ItemModel &operator=(ItemModel &&) noexcept = default;  // Move assignment operator

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

        [[nodiscard]] QuerySet qsCount() override;

        [[nodiscard]] std::vector<
            std::pair<BaseField,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
        getObjectValues() const;
        [[nodiscard]] std::string
        sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params) override;
        [[nodiscard]] std::string sqlSelectOne(const std::string &field,
                                               const std::string &value,
                                               const std::map<std::string, std::string, std::less<>> &params) override;
        [[nodiscard]] std::map<std::string, std::pair<std::string, std::string>, std::less<>> joinMap() const override;
    };
}

#endif  //ITEMMODEL_H
