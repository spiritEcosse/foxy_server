#pragma once

#include "BaseTestClass.h"
#include "Country.h"

#include <gtest/gtest.h>

class CountryControllerTest : public BaseTestClass<CountryControllerTest, api::v1::Country> {
public:
    static FieldsMap expectedValues;
    static FieldsMap updatedValues;
    static FieldsMap getOneValues;
};

FieldsMap CountryControllerTest::expectedValues = {
    {"title", "mock title"},
    {"code", "mock code"},
};
FieldsMap CountryControllerTest::updatedValues = {};
FieldsMap CountryControllerTest::getOneValues = {};

TEST_F(CountryControllerTest, Create200) {
    testCreate200();
}

TEST_F(CountryControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(CountryControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(CountryControllerTest, Delete204) {
    testDelete204();
}

TEST_F(CountryControllerTest, Update200) {
    testUpdate200();
}
