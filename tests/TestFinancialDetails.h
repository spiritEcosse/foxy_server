#pragma once

#include "BaseTestClass.h"
#include "FinancialDetails.h"

#include <gtest/gtest.h>
#include <drogon/drogon.h>

class FinancialDetailsControllerTest : public BaseTestClass<FinancialDetailsControllerTest, api::v1::FinancialDetails> {
public:
    static constexpr drogon::HttpMethod method = drogon::Post;
    static FieldsMap expectedValues;
};

FieldsMap FinancialDetailsControllerTest::expectedValues = {
    {"description", "mock description"},
    {"meta_description", "mock meta description"},
    {"price", 100.0},
    {"shipping_profile_id", 1},
    {"slug", "mock-slug"},
    {"title", "mock title"},
    {"enabled", true},
};

TEST_F(FinancialDetailsControllerTest, Create200) {
    testCreate200();
}

TEST_F(FinancialDetailsControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(FinancialDetailsControllerTest, RequiredFields400) {
    testRequiredFields400();
}
