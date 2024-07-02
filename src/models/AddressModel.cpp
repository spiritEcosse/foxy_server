#include "AddressModel.h"
#include "CountryModel.h"

namespace api::v1 {

    std::vector<BaseField> AddressModel::fields() {
        return {Field::address, Field::countryId, Field::city, Field::zipcode, Field::userId};
    }

    std::vector<std::pair<BaseField, std::variant<int, bool, std::string, std::chrono::system_clock::time_point>>>
    AddressModel::getObjectValues() const {
        return {{Field::address, address},
                {Field::countryId, countryId},
                {Field::city, city},
                {Field::zipcode, zipcode},
                {Field::userId, userId}};
    }

    std::string
    AddressModel::sqlSelectList(int page, int limit, const std::map<std::string, std::string, std::less<>> &params) {
        QuerySet qsCount = AddressModel().qsCount();
        QuerySet qsPage = AddressModel().qsPage(page, limit);
        QuerySet qs(tableName, limit, "data");
        qs.join(CountryModel())
            .only(allSetFields())
            .functions(
                Function(std::format(R"(, json_build_object({}) AS country)", CountryModel().fieldsJsonObject())));
        return QuerySet::buildQuery(std::move(qsCount), std::move(qsPage), std::move(qs));
    }

}  // namespace api::v1
