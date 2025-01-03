#pragma once

#include "BaseTestClass.h"
#include "Item.h"

#include <gtest/gtest.h>

class ItemControllerTest : public BaseTestClass<ItemControllerTest, api::v1::Item> {
    void SetUp() override {
        expectedValues["description"] = "mock description";
        expectedValues["meta_description"] = "mock meta description";
        expectedValues["price"] = 100.0;
        expectedValues["shipping_profile_id"] = 1;
        expectedValues["slug"] = "mock-slug";
        expectedValues["title"] = "mock title";
        expectedValues["enabled"] = true;

        updatedValues["description"] = "new mock description";
        updatedValues["meta_description"] = "new mock meta description";
        updatedValues["price"] = 200.0;
        updatedValues["shipping_profile_id"] = 2;
        updatedValues["slug"] = "new-mock-slug";
        updatedValues["title"] = "new mock title";
        updatedValues["enabled"] = false;
        BaseTestClass::SetUp();
    }
};

TEST_F(ItemControllerTest, Create200) {
    testCreate200();
}

TEST_F(ItemControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(ItemControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(ItemControllerTest, Delete204) {
    testDelete204();
}

TEST_F(ItemControllerTest, Update200) {
    testUpdate200();
}

TEST_F(ItemControllerTest, GetOne200) {
    getOne200();
}
