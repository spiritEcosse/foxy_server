#pragma once

#include <string>
#include "models/BaseModel.h"

namespace api::v1 {

    class CollectionItemModel final : public BaseModel<CollectionItemModel> {
    public:
        using BaseModel::BaseModel;

        static const inline std::string tableName = "collection_item";

        struct Field : BaseModel::Field {
            static inline const auto collectionId = BaseField("collection_id", tableName);
            static inline const auto itemId = BaseField("item_id", tableName);

            Field() : BaseModel::Field() {
                constexpr std::array fields{&collectionId, &itemId};
                registerFields(fields);
            }
        };

        int collectionId{};
        int itemId{};

        explicit CollectionItemModel(const Json::Value &json) : BaseModel(json) {
            collectionId = json[Field::collectionId.getFieldName()].asInt();
            itemId = json[Field::itemId.getFieldName()].asInt();

            validateField(Field::collectionId.getFieldName(), collectionId, missingFields);
            validateField(Field::itemId.getFieldName(), itemId, missingFields);
        }

        [[nodiscard]] SetMapFieldTypes getObjectValues() const;
        [[nodiscard]] static JoinMap joinMap();
    };
}
