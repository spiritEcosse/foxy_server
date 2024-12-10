#include "AddressModel.h"
#include "CountryModel.h"
#include "OrderModel.h"

namespace api::v1 {

    std::map<std::string, std::pair<std::string, std::string>, std::less<>> AddressModel::joinMap() const {
        return {{CountryModel::tableName,
                 {Field::countryId.getFullFieldName(), BaseModel<CountryModel>::Field::id.getFullFieldName()}},
                {OrderModel::tableName,
                 {BaseModel::Field::id.getFullFieldName(), OrderModel::Field::addressId.getFullFieldName()}}};
    }

    BaseModel<AddressModel>::SetMapFieldTypes AddressModel::getObjectValues() const {
        return {{std::cref(Field::address), std::cref(address)},
                {std::cref(Field::countryId), std::cref(countryId)},
                {std::cref(Field::city), std::cref(city)},
                {std::cref(Field::zipcode), std::cref(zipcode)},
                {std::cref(Field::userId), std::cref(userId)}};
    }

    std::string AddressModel::sqlSelectList(const int page,
                                            int limit,
                                            const std::map<std::string, std::string, std::less<>> &params) {
        QuerySet qsCount = AddressModel::qsCount();
        QuerySet qsPage = AddressModel::qsPage(page, limit);
        QuerySet qs(tableName, limit, "data");
        qs.join(CountryModel())
            .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
            .only(allSetFields())
            .order_by(std::make_pair(std::cref(BaseModel::Field::updatedAt), false))
            .functions(Function(fmt::format(R"(json_build_object({}) AS country)", CountryModel().fieldsJsonObject())));
        applyFilters(qs, qsCount, params);
        return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
    }

    std::string AddressModel::fieldsJsonObject() {
        std::string str = BaseModel::fieldsJsonObject();
        QuerySet qs(CountryModel::tableName, "country", false, false);
        qs.jsonFields(CountryModel().fieldsJsonObject())
            .filter(BaseModel<CountryModel>::Field::id.getFullFieldName(),
                    Field::countryId.getFullFieldName(),
                    false,
                    std::string("="));
        std::string sql = qs.buildSelect();
        str += fmt::format(", 'country', ({})", sql);
        return str;
    }

}
