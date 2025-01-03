#pragma once

#include "BaseTestClass.h"
#include "ShippingProfile.h"

#include <gtest/gtest.h>

class ShippingProfileControllerTest : public BaseTestClass<ShippingProfileControllerTest, api::v1::ShippingProfile> {
    void SetUp() override {
        expectedValues["title"] = "mock title";
        expectedValues["processing_time"] = 1;
        expectedValues["country_id"] = 1;
        expectedValues["postal_code"] = "mock postal code";

        updatedValues["title"] = "new mock title";
        updatedValues["processing_time"] = 2;
        updatedValues["country_id"] = 2;
        updatedValues["postal_code"] = "new mock postal code";
        BaseTestClass::SetUp();
    }
};

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
