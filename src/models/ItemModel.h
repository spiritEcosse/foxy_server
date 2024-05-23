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
            static inline BaseField<ItemModel> title = BaseField<ItemModel>("title");
            static inline BaseField<ItemModel> description = BaseField<ItemModel>("description");
            static inline BaseField<ItemModel> metaDescription = BaseField<ItemModel>("meta_description");
            static inline BaseField<ItemModel> shippingProfileId = BaseField<ItemModel>("shipping_profile_id");
            static inline BaseField<ItemModel> slug = BaseField<ItemModel>("slug");
            static inline BaseField<ItemModel> enabled = BaseField<ItemModel>("enabled");
            static inline BaseField<ItemModel> price = BaseField<ItemModel>("price");
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
            auto priceString = json[Field::price.getFieldName()].asString();

            validateField(Field::title.getFieldName(), title, missingFields);
            validateField(Field::description.getFieldName(), description, missingFields);
            validateField(Field::shippingProfileId.getFieldName(), shippingProfileId, missingFields);
            validateField(Field::metaDescription.getFieldName(), metaDescription, missingFields);
            validateField(Field::slug.getFieldName(), slug, missingFields);
            validateField(Field::price.getFieldName(), priceString, missingFields);
            if(missingFields.empty()) {
                price = std::stod(priceString);
            }
        }

        [[nodiscard]] static QuerySet qsCount();

        [[nodiscard]] static std::vector<BaseField<ItemModel>> fields();
        [[nodiscard]] static std::vector<BaseField<ItemModel>> fullFields();
        [[nodiscard]] std::vector<
            std::pair<BaseField<ItemModel>,
                      std::variant<int, bool, std::string, std::chrono::system_clock::time_point, dec::decimal<2>>>>
        getObjectValues() const;
        [[nodiscard]] static std::string sqlSelectList(int page, int limit);
        [[nodiscard]] static std::string
        sqlSelectOne(const std::string &field,
                     const std::string &value,
                     const std::map<std::string, std::string, std::less<>> &params = {});
    };
}

#endif  //ITEMMODEL_H
