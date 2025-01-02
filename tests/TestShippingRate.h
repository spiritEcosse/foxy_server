#pragma once

#include "BaseTestClass.h"
#include "ShippingRate.h"

#include <gtest/gtest.h>
#include <drogon/drogon.h>

class ShippingRateControllerTest : public BaseTestClass<ShippingRateControllerTest, api::v1::ShippingRate> {
public:
    static constexpr drogon::HttpMethod method = drogon::Post;
    static FieldsMap expectedValues;
};

FieldsMap ShippingRateControllerTest::expectedValues = {
    {"shipping_profile_id", 1},
    {"delivery_days_min", 1},
    {"delivery_days_max", 1},
};

TEST_F(ShippingRateControllerTest, Create200) {
    testCreate200();
}

TEST_F(ShippingRateControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(ShippingRateControllerTest, RequiredFields400) {
    testRequiredFields400();
}
