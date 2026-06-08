#pragma once

#include <string>
#include "models/BaseModel.h"

namespace api::v1 {

    class CollectionModel final : public BaseModel<CollectionModel> {
    public:
        using BaseModel::BaseModel;
        static const inline std::string tableName = "collection";

        struct Field : BaseModel::Field {
            static inline const auto title = BaseField("title", tableName);
            static inline const auto slug = BaseField("slug", tableName);
            static inline const auto description = BaseField("description", tableName);
            static inline const auto metaDescription = BaseField("meta_description", tableName);
            static inline const auto enabled = BaseField("enabled", tableName);

            Field() : BaseModel::Field() {
                constexpr std::array fields{&title, &slug, &description, &metaDescription, &enabled};
                registerFields(fields);
            }
        };

        std::string title;
        std::string slug;
        std::string description;
        std::string metaDescription;
        bool enabled = false;

        explicit CollectionModel(const Json::Value &json) : BaseModel(json) {
            title = json[Field::title.getFieldName()].asString();
            slug = json[Field::slug.getFieldName()].asString();
            description = json[Field::description.getFieldName()].asString();
            metaDescription = json[Field::metaDescription.getFieldName()].asString();
            enabled = json[Field::enabled.getFieldName()].asBool();

            validateField(Field::title.getFieldName(), title, missingFields);
            validateField(Field::slug.getFieldName(), slug, missingFields);
            validateField(Field::description.getFieldName(), description, missingFields);
            validateField(Field::metaDescription.getFieldName(), metaDescription, missingFields);
        }

        [[nodiscard]] static QuerySet<CollectionModel> qsCount();

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
        [[nodiscard]] static std::string
        sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params);
        [[nodiscard]] std::string sqlSelectOne(const BaseField *field,
                                               std::string &&value,
                                               const std::map<std::string, std::string, std::less<>> &params) override;
        [[nodiscard]] static JoinMap joinMap();
    };
}
