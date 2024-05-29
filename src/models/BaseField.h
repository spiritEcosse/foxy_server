#include <string>
#include <fmt/core.h>

namespace api::v1
{
class BaseField
{
public:
    explicit BaseField() = default;
    explicit BaseField(std::string fieldName, std::string tableName)
        : fieldName(std::move(fieldName)), tableName(std::move(tableName))
    {
    }

    [[nodiscard]] std::string getFieldName() const
    {
        return fieldName;
    }

    [[nodiscard]] std::string getFullFieldName() const
    {
        return fmt::format(R"("{}"."{}")", tableName, fieldName);
    }

    [[nodiscard]] bool empty() const
    {
        return fieldName.empty();
    }
private:
    std::string fieldName;
    std::string tableName;
};
}
