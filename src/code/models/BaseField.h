#pragma once

#include <string>
#include "BaseClass.h"
#include <fmt/core.h>

namespace api::v1 {
    class BaseField final : public BaseClass {
    public:
        using BaseClass::BaseClass;

        explicit BaseField(std::string fieldName, const std::string_view& tableName) :
            fieldName(std::move(fieldName)), tableName(tableName) {}

        [[nodiscard]] std::string getFieldName() const {
            return fieldName;
        }

        [[nodiscard]] std::string getFullFieldName() const {
            return fmt::format(R"("{}"."{}")", tableName, fieldName);
        }

        [[nodiscard]] bool empty() const {
            return fieldName.empty();
        }

    private:
        std::string fieldName;
        std::string tableName;
    };
}
