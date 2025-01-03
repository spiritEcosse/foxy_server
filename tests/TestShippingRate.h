#pragma once

#include "BaseTestClass.h"
#include "ShippingRate.h"

#include <gtest/gtest.h>

class ShippingRateControllerTest : public BaseTestClass<ShippingRateControllerTest, api::v1::ShippingRate> {
public:
    static FieldsMap expectedValues;
    static FieldsMap updatedValues;
    static FieldsMap getOneValues;
};

FieldsMap ShippingRateControllerTest::expectedValues = {
    {"shipping_profile_id", 1},
    {"delivery_days_min", 1},
    {"delivery_days_max", 1},
};
FieldsMap ShippingRateControllerTest::updatedValues = {};
FieldsMap ShippingRateControllerTest::getOneValues = {};

TEST_F(ShippingRateControllerTest, Create200) {
    testCreate200();
}

TEST_F(ShippingRateControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(ShippingRateControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(ShippingRateControllerTest, Delete204) {
    testDelete204();
}

TEST_F(ShippingRateControllerTest, Update200) {
    testUpdate200();
}
