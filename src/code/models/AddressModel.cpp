#include "AddressModel.h"
#include "CountryModel.h"
#include "OrderModel.h"

namespace api::v1 {

    BaseModelImpl::JoinMap AddressModel::joinMap() const {
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
        QuerySet qsCount = AddressModel::qsCount();
        QuerySet qsPage = AddressModel::qsPage(page, limit);
        QuerySet qs(tableName, limit, "data");
        qs.join(CountryModel())
            .offset(fmt::format("((SELECT * FROM {}) - 1) * {}", qsPage.alias(), limit))
            .only(allSetFields())
            .order_by(std::make_pair(&BaseModel::Field::updatedAt, false))
            .functions(Function(fmt::format(R"(json_build_object({}) AS country)", CountryModel().fieldsJsonObject())));
        applyFilters(qs, qsCount, params);
        return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
    }

    std::string AddressModel::fieldsJsonObject() {
        std::string str = BaseModel::fieldsJsonObject();
        QuerySet qs(CountryModel::tableName, "country", false, false);
        qs.jsonFields(CountryModel().fieldsJsonObject()).filter(&BaseModel<CountryModel>::Field::id, &Field::countryId);
        std::string sql = qs.buildSelect();
        str += fmt::format(", 'country', ({})", sql);
        return str;
    }

}
