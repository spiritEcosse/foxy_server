#pragma once

#include "BaseTestClass.h"
#include "ShippingProfile.h"

#include <gtest/gtest.h>

class ShippingProfileControllerTest : public BaseTestClass<ShippingProfileControllerTest, api::v1::ShippingProfile> {
    void setupExpectedValues() override {
        expectedValues["title"] = "mock title";
        expectedValues["processing_time"] = 1;
        expectedValues["country_id"] = 1;
        expectedValues["postal_code"] = "mock postal code";
    }

    void setupUpdatedValues() override {
        updatedValues["title"] = "new mock title";
        updatedValues["processing_time"] = 2;
        updatedValues["country_id"] = 2;
        updatedValues["postal_code"] = "new mock postal code";
    }

    void setupGetOneValues() override {
        getOneValues["country_id"] = 1;
        getOneValues["id"] = 1;
        getOneValues["postal_code"] = "12345";
        getOneValues["processing_time"] = 2;
        getOneValues["shipping_upgrade_cost"] = 10.0;
        getOneValues["title"] = "Profile1";
    }

    void setupGetListValues() override {
        getListValues["_page"] = 1;
        getListValues["total"] = 2;

        Json::Value data = Json::arrayValue;

        Json::Value entry1;
        entry1["country_id"] = 2;
        entry1["id"] = 2;
        entry1["postal_code"] = "67890";
        entry1["processing_time"] = 3;
        entry1["shipping_upgrade_cost"] = 15.0;
        entry1["title"] = "Profile2";

        Json::Value entry2;
        entry2["country_id"] = 1;
        entry2["id"] = 1;
        entry2["postal_code"] = "12345";
        entry2["processing_time"] = 2;
        entry2["shipping_upgrade_cost"] = 10.0;
        entry2["title"] = "Profile1";

        data.append(entry1);
        data.append(entry2);

        getListValues["data"] = data;
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

TEST_F(ShippingProfileControllerTest, GetOne200) {
    getOne200();
}

TEST_F(ShippingProfileControllerTest, GetList200) {
    getList200();
}
