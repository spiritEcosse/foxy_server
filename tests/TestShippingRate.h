#pragma once

#include "BaseTestClass.h"
#include "ShippingRate.h"

#include <gtest/gtest.h>

class ShippingRateControllerTest : public BaseTestClass<ShippingRateControllerTest, api::v1::ShippingRate> {
    void setupExpectedValues() override {
        expectedValues["shipping_profile_id"] = 1;
        expectedValues["delivery_days_min"] = 1;
        expectedValues["delivery_days_max"] = 1;
    }

    void setupUpdatedValues() override {
        updatedValues["shipping_profile_id"] = 2;
        updatedValues["delivery_days_min"] = 2;
        updatedValues["delivery_days_max"] = 2;
    }

    void setupGetOneValues() override {
        getOneValues["country_id"] = 1;
        getOneValues["delivery_days_max"] = 5;
        getOneValues["delivery_days_min"] = 1;
        getOneValues["id"] = 1;
        getOneValues["shipping_profile_id"] = 1;
    }
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

TEST_F(ShippingRateControllerTest, Delete204) {
    testDelete204();
}

TEST_F(ShippingRateControllerTest, Update200) {
    testUpdate200();
}

TEST_F(ShippingRateControllerTest, GetOne200) {
    getOne200();
}