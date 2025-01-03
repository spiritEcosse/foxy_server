#pragma once

#include "BaseTestClass.h"
#include "ShippingProfile.h"

#include <gtest/gtest.h>

class ShippingProfileControllerTest : public BaseTestClass<ShippingProfileControllerTest, api::v1::ShippingProfile> {
public:
    static FieldsMap expectedValues;
    static FieldsMap updatedValues;
};

FieldsMap ShippingProfileControllerTest::expectedValues = {
    {"title", "mock title"},
    {"processing_time", 1},
    {"country_id", 1},
    {"postal_code", "mock postal code"},
};
FieldsMap ShippingProfileControllerTest::updatedValues = {};

TEST_F(ShippingProfileControllerTest, Create200) {
    testCreate200();
}

TEST_F(ShippingProfileControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(ShippingProfileControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(ShippingProfileControllerTest, Delete204) {
    testDelete204();
}

TEST_F(ShippingProfileControllerTest, Update200) {
    testUpdate200();
}
