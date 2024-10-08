#ifndef BASEMODEL_H
#define BASEMODEL_H

#include <unordered_map>
#include "BaseModelImpl.h"

namespace api::v1 {

    template<class T>
    class BaseModel : public BaseModelImpl {
    public:
        static inline const std::string tableName;

        struct Field {
            static inline BaseField id = BaseField("id", T::tableName);
            static inline BaseField createdAt = BaseField("created_at", T::tableName);
            static inline BaseField updatedAt = BaseField("updated_at", T::tableName);
            std::map<std::string, BaseField, std::less<>> allFields = {
                {id.getFieldName(), id},
                {createdAt.getFieldName(), createdAt},
                {updatedAt.getFieldName(), updatedAt},
            };
        };

        BaseModel() = default;
        BaseModel(const BaseModel &) = delete;
        BaseModel &operator=(const BaseModel &) = delete;
        BaseModel(BaseModel &&) noexcept = default;
        BaseModel &operator=(BaseModel &&) noexcept = default;
        virtual ~BaseModel() = default;

        Json::Value missingFields;
        std::chrono::system_clock::time_point updatedAt;
        std::chrono::system_clock::time_point createdAt;
        int id = 0;

        explicit BaseModel(const Json::Value &json);

        using ModelFieldKeyHash =
            decltype(std::unordered_map<std::string, std::string, ModelFieldHasher, std::equal_to<>>());

        [[nodiscard]] virtual std::string sqlInsertMultiple(const std::vector<T> &item);
        [[nodiscard]] virtual std::string sqlInsertSingle(const T &item);
        [[nodiscard]] virtual std::string sqlInsert(const T &item);
        [[nodiscard]] virtual std::string sqlUpdateMultiple(const std::vector<T> &item);
        [[nodiscard]] virtual QuerySet qsCount();
        [[nodiscard]] virtual QuerySet qsPage(int page, int limit);
        virtual void sqlUpdateSingle(const T &item, ModelFieldKeyHash &uniqueColumns);
        [[nodiscard]] virtual std::string sqlUpdate(T &&item);
        [[nodiscard]] virtual std::string
        sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params);
        [[nodiscard]] virtual std::string sqlSelectOne(const std::string &field,
                                                       const std::string &value,
                                                       const std::map<std::string, std::string, std::less<>> &params);
        [[nodiscard]] virtual std::string fieldsToString();
        [[nodiscard]] virtual std::string fieldsJsonObject();
        [[nodiscard]] virtual std::string sqlDelete(int id);
        [[nodiscard]] virtual std::string sqlDeleteMultiple(const std::vector<int> &ids);

        [[nodiscard]] bool fieldExists(const std::string &fieldName) const;
        [[nodiscard]] std::vector<BaseField> allSetFields() const;
        virtual void applyFilters(QuerySet &qs,
                                  QuerySet &qsCount,
                                  const std::map<std::string, std::string, std::less<>> &params) const;
    };
}

#include "BaseModel.inl"

#endif  //BASEMODEL_H