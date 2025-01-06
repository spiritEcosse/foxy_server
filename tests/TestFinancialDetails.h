#pragma once

#include "BaseTestClass.h"
#include "FinancialDetails.h"

#include <gtest/gtest.h>

class FinancialDetailsControllerTest : public BaseTestClass<FinancialDetailsControllerTest, api::v1::FinancialDetails> {
    void setupExpectedValues() override {
        expectedValues["description"] = "mock description";
        expectedValues["meta_description"] = "mock meta description";
        expectedValues["price"] = 100.0;
        expectedValues["shipping_profile_id"] = 1;
        expectedValues["slug"] = "mock-slug";
        expectedValues["title"] = "mock title";
        expectedValues["enabled"] = true;
    }

    void setupUpdatedValues() override {
        updatedValues["description"] = "new mock description";
        updatedValues["meta_description"] = "new mock meta description";
        updatedValues["price"] = 200.0;
        updatedValues["shipping_profile_id"] = 2;
        updatedValues["slug"] = "new-mock-slug";
        updatedValues["title"] = "new mock title";
        updatedValues["enabled"] = false;
    }

    void setupGetOneValues() override {
        getOneValues["title"] = "Country1";
        getOneValues["code"] = "C1";
    }
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

TEST_F(FinancialDetailsControllerTest, Delete204) {
    testDelete204();
}

TEST_F(FinancialDetailsControllerTest, Update200) {
    testUpdate200();
}

TEST_F(FinancialDetailsControllerTest, GetOne200) {
    getOne200();
}
