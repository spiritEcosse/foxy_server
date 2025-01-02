#pragma once

#include "BaseTestClass.h"
#include "Country.h"

#include <gtest/gtest.h>

class CountryControllerTest : public BaseTestClass<CountryControllerTest, api::v1::Country> {
public:
    static constexpr drogon::HttpMethod method = drogon::Post;
    static FieldsMap expectedValues;
};

FieldsMap CountryControllerTest::expectedValues = {
    {"title", "mock title"},
    {"code", "mock code"},
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
