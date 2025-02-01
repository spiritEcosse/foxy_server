#include "AddressModel.h"
#include "CountryModel.h"
#include "OrderModel.h"

namespace api::v1 {

    BaseModelImpl::JoinMap AddressModel::joinMap() {
        return {{CountryModel::tableName, {&Field::countryId, &BaseModel<CountryModel>::Field::id}},
                {OrderModel::tableName, {&BaseModel::Field::id, &OrderModel::Field::addressId}}};
    }

    BaseModel<AddressModel>::SetMapFieldTypes AddressModel::getObjectValues() const {
        return {{&Field::address, address},
                {&Field::countryId, countryId},
                {&Field::city, city},
                {&Field::zipcode, zipcode},
                {&Field::userId, userId}};
    }

    std::string AddressModel::sqlSelectList(const int page,
                                            int limit,
                                            const std::map<std::string, std::string, std::less<>> &params) {
        auto qsCount = AddressModel::qsCount();
        auto qsPage = AddressModel::qsPage(page, limit);
        QuerySet<AddressModel> qs(limit, "data");
        qs.join<CountryModel>()
            .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
            .only(allSetFields())
            .order_by(&BaseModel::Field::updatedAt, false)
            .functions(Function(fmt::format(R"(json_build_object({}) AS country)", CountryModel().fieldsJsonObject())));
        applyFilters(qs, qsCount, params);
        return BuildComplexQueries::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
    }

    std::string AddressModel::fieldsJsonObject() {
        std::string str = BaseModel::fieldsJsonObject();
        QuerySet<CountryModel> qs("country", false, false);
        qs.jsonFields(CountryModel().fieldsJsonObject()).filter(&BaseModel<CountryModel>::Field::id, &Field::countryId);
        std::string sql = qs.buildSelect();
        str += fmt::format(", 'country', ({})", sql);
        return str;
    }

}
