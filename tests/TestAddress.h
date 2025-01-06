#pragma once

#include "BaseTestClass.h"
#include "Address.h"

#include <gtest/gtest.h>

class AddressControllerTest : public BaseTestClass<AddressControllerTest, api::v1::Address> {
    void setupExpectedValues() override {
        expectedValues["address"] = "mock address";
        expectedValues["country_id"] = 1;
        expectedValues["city"] = "mock city";
        expectedValues["zipcode"] = "mock zipcode";
        expectedValues["user_id"] = 1;
    }

    void setupUpdatedValues() override {
        updatedValues["address"] = "new mock address";
        updatedValues["country_id"] = 2;
        updatedValues["city"] = "new mock city";
        updatedValues["zipcode"] = "new mock zipcode";
        updatedValues["user_id"] = 2;
    }

    void setupGetOneValues() override {
        getOneValues["address"] = "Address1";
        getOneValues["country_id"] = 1;
        getOneValues["city"] = "City1";
        getOneValues["zipcode"] = "12345";
        getOneValues["user_id"] = 1;
        getOneValues["id"] = 1;
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

TEST_F(AddressControllerTest, GetOne200) {
    getOne200();
}
