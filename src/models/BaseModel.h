#pragma once

#include <unordered_map>
#include "BaseModelImpl.h"

namespace api::v1 {

    template<class T>
    class BaseModel : public BaseModelImpl {
    public:
        using BaseModelImpl::BaseModelImpl;
        static const inline std::string tableName;

        struct Field {
            static inline auto id = BaseField("id", T::tableName);
            static inline auto createdAt = BaseField("created_at", T::tableName);
            static inline auto updatedAt = BaseField("updated_at", T::tableName);
            std::map<std::string, std::reference_wrapper<const BaseField>, std::less<>> allFields = {
                {id.getFieldName(), std::cref(id)},
                {createdAt.getFieldName(), std::cref(createdAt)},
                {updatedAt.getFieldName(), std::cref(updatedAt)}};
        };

        Json::Value missingFields;
        std::chrono::system_clock::time_point updatedAt;
        std::chrono::system_clock::time_point createdAt;
        int id = 0;

        explicit BaseModel(const Json::Value &json);

        using ModelFieldKeyHash =
            decltype(std::unordered_map<std::string, std::string, ModelFieldHasher, std::equal_to<>>());

        using MapFieldTypes = std::pair<std::reference_wrapper<const BaseField>,
                                        std::variant<int,
                                                     bool,
                                                     std::vector<std::string>,
                                                     std::string,
                                                     std::chrono::system_clock::time_point,
                                                     dec::decimal<2>>>;
        using SetMapFieldTypes = std::vector<MapFieldTypes>;
        [[nodiscard]] virtual std::string sqlInsertMultiple(const std::vector<T> &items);
        [[nodiscard]] virtual std::string sqlInsertSingle(const T &item);
        [[nodiscard]] virtual std::string sqlInsert(const T &item);
        [[nodiscard]] virtual std::string sqlUpdateMultiple(const std::vector<T> &items);
        [[nodiscard]] static QuerySet qsCount();
        [[nodiscard]] static QuerySet qsPage(int page, int limit);
        virtual void sqlUpdateSingle(const T &item, ModelFieldKeyHash &uniqueColumns);
        [[nodiscard]] virtual std::string sqlUpdate(T &&item);
        [[nodiscard]] static std::string
        sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params);
        [[nodiscard]] virtual std::string sqlSelectOne(const std::string &field,
                                                       const std::string &value,
                                                       const std::map<std::string, std::string, std::less<>> &params);
        [[nodiscard]] virtual std::string fieldsToString();
        [[nodiscard]] virtual std::string fieldsJsonObject();
        [[nodiscard]] virtual std::string sqlDelete(int id);
        [[nodiscard]] virtual std::string sqlDeleteMultiple(const std::vector<int> &ids);

        [[nodiscard]] static std::vector<std::reference_wrapper<const BaseField>> allSetFields();
        static void
        applyFilters(QuerySet &qs, QuerySet &qsCount, const std::map<std::string, std::string, std::less<>> &params);
    };
}

#include "BaseModel.inl"
