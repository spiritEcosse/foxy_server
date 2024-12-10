#include "PageModel.h"

using namespace api::v1;

BaseModel<PageModel>::SetMapFieldTypes PageModel::getObjectValues() const {
    return {{std::cref(Field::title), title},
            {std::cref(Field::description), description},
            {std::cref(Field::metaDescription), metaDescription},
            {std::cref(Field::canonicalUrl), canonicalUrl},
            {std::cref(Field::slug), slug},
            {std::cref(Field::enabled), enabled}};
}
