#pragma once
#include <gmock/gmock.h>
#include <cpr/cpr.h>
#include "HttpRequestInterface.h"

namespace api::v1 {
    class MockHttpClient final : public HttpRequestInterface {
    public:
        MOCK_METHOD(cpr::Response, implGet, (std::string url), (override));
    };
}
