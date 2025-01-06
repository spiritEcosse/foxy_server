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

    void setupGetListValues() override {
        getListValues["_page"] = 1;
        getListValues["total"] = 2;

        Json::Value data = Json::arrayValue;

        Json::Value entry1;
        entry1["country_id"] = 2;
        entry1["id"] = 2;
        entry1["delivery_days_max"] = 6;
        entry1["delivery_days_min"] = 2;
        entry1["shipping_profile_id"] = 2;

        Json::Value entry2;
        entry2["country_id"] = 1;
        entry2["id"] = 1;
        entry2["delivery_days_max"] = 5;
        entry2["delivery_days_min"] = 1;
        entry2["shipping_profile_id"] = 1;

        data.append(entry1);
        data.append(entry2);

        getListValues["data"] = data;
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

TEST_F(ShippingRateControllerTest, GetList200) {
    getList200();
}
