#pragma once
#include "BaseClass.h"

#include <cpr/cpr.h>

namespace api::v1 {
    class HttpRequestInterface : public BaseClass {
    public:
        virtual cpr::Response implGet(std::string url) = 0;
    };
}
