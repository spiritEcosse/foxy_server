#pragma once

#include <unordered_map>
#include "BaseModelImpl.h"
#include <algorithm>

namespace api::v1 {

    template<class T>
    class BaseModel : public BaseModelImpl {
    public:
        using BaseModelImpl::BaseModelImpl;
        static const inline std::string tableName;

        struct Field {
            static inline const auto id = BaseField("id", T::tableName);
            static inline const auto createdAt = BaseField("created_at", T::tableName);
            static inline const auto updatedAt = BaseField("updated_at", T::tableName);
            AllFields allFields = {{id.getFieldName(), &id},
                                   {createdAt.getFieldName(), &createdAt},
                                   {updatedAt.getFieldName(), &updatedAt}};

            template<std::size_t N>
            void registerFields(const std::array<const BaseField *, N> &fields) {
                std::ranges::for_each(fields, [this](const BaseField *field) {
                    allFields.try_emplace(field->getFieldName(), field);
                });
            }
        };

        Json::Value missingFields;
        std::chrono::system_clock::time_point updatedAt;
        std::chrono::system_clock::time_point createdAt;
        int id = 0;

        explicit BaseModel(const Json::Value &json);

        using MapFieldTypes = std::pair<const BaseField *,
                                        std::variant<int,
                                                     bool,
                                                     std::vector<std::string>,
                                                     std::string,
                                                     std::nullopt_t,
                                                     std::chrono::system_clock::time_point,
                                                     dec::decimal<2>>>;
        using SetMapFieldTypes = std::vector<MapFieldTypes>;
        [[nodiscard]] virtual std::string sqlInsertMultiple(const std::vector<T> &items);
        [[nodiscard]] virtual std::string sqlInsertSingle(const T &item);
        [[nodiscard]] virtual std::string sqlInsert(const T &item);
        [[nodiscard]] virtual std::string sqlUpdateMultiple(const std::vector<T> &items);
        [[nodiscard]] static QuerySet qsCount();
        [[nodiscard]] static QuerySet qsPage(int page, int limit);
        virtual void sqlUpdateSingle(const T &item, TransparentMap &uniqueColumns);
        [[nodiscard]] virtual std::string sqlUpdate(T &&item);
        [[nodiscard]] static std::string
        sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params);
        [[nodiscard]] virtual std::string sqlSelectOne(const BaseField *field,
                                                       const std::string &value,
                                                       const std::map<std::string, std::string, std::less<>> &params);
        [[nodiscard]] virtual std::string fieldsToString();
        [[nodiscard]] virtual std::string fieldsJsonObject();
        [[nodiscard]] virtual std::string sqlDelete(int id);
        [[nodiscard]] virtual std::string sqlDeleteMultiple(const std::vector<int> &ids);

        [[nodiscard]] static std::vector<const BaseField *> allSetFields();
        static void
        applyFilters(QuerySet &qs, QuerySet &qsCount, const std::map<std::string, std::string, std::less<>> &params);
    };
}

#include "BaseModel.inl"
