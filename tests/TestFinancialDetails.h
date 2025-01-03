#pragma once

#include "BaseTestClass.h"
#include "FinancialDetails.h"

#include <gtest/gtest.h>

class FinancialDetailsControllerTest : public BaseTestClass<FinancialDetailsControllerTest, api::v1::FinancialDetails> {
public:
    static FieldsMap expectedValues;
    static FieldsMap updatedValues;
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
FieldsMap FinancialDetailsControllerTest::updatedValues = {};

TEST_F(FinancialDetailsControllerTest, Create200) {
    testCreate200();
}

TEST_F(FinancialDetailsControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(FinancialDetailsControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(FinancialDetailsControllerTest, Delete204) {
    testDelete204();
}

TEST_F(FinancialDetailsControllerTest, Update200) {
    testUpdate200();
}
