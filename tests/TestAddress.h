#pragma once

#include "BaseTestClass.h"
#include "Address.h"

#include <gtest/gtest.h>

class AddressControllerTest : public BaseTestClass<AddressControllerTest, api::v1::Address> {
public:
    static FieldsMap expectedValues;
    static FieldsMap updatedValues;
    static FieldsMap getOneValues;
};

FieldsMap AddressControllerTest::expectedValues = {
    {"address", "mock address"},
    {"country_id", 1},
    {"city", "mock city"},
    {"zipcode", "mock zipcode"},
    {"user_id", 1},
};
FieldsMap AddressControllerTest::updatedValues = {};
FieldsMap AddressControllerTest::getOneValues = {};

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
