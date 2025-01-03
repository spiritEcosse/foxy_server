#pragma once

#include "BaseTestClass.h"
#include "Country.h"

#include <gtest/gtest.h>

class CountryControllerTest : public BaseTestClass<CountryControllerTest, api::v1::Country> {
    void SetUp() override {
        expectedValues["title"] = "mock title";
        expectedValues["code"] = "mock code";

        updatedValues["title"] = "new mock title";
        updatedValues["code"] = "new mock code";

        BaseTestClass::SetUp();
    }
};

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
