#include "PageModel.h"

using namespace api::v1;

BaseModel<PageModel>::SetMapFieldTypes PageModel::getObjectValues() const {
    return {{&Field::title, title},
            {&Field::description, description},
            {&Field::metaDescription, metaDescription},
            {&Field::canonicalUrl, canonicalUrl},
            {&Field::slug, slug},
            {&Field::enabled, enabled}};
}
