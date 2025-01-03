#pragma once

#include "BaseTestClass.h"
#include "Address.h"

#include <gtest/gtest.h>

class AddressControllerTest : public BaseTestClass<AddressControllerTest, api::v1::Address> {
    void SetUp() override {
        expectedValues["address"] = "mock address";
        expectedValues["country_id"] = 1;
        expectedValues["city"] = "mock city";
        expectedValues["zipcode"] = "mock zipcode";
        expectedValues["user_id"] = 1;

        updatedValues["address"] = "new mock address";
        updatedValues["country_id"] = 2;
        updatedValues["city"] = "new mock city";
        updatedValues["zipcode"] = "new mock zipcode";
        updatedValues["user_id"] = 2;

        BaseTestClass::SetUp();
    }
};

TEST_F(AddressControllerTest, Create200) {
    testCreate200();
}

TEST_F(AddressControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(AddressControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(AddressControllerTest, Delete204) {
    testDelete204();
}

TEST_F(AddressControllerTest, Update200) {
    testUpdate200();
}
