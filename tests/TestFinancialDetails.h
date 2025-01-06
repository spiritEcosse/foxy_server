#pragma once

#include "BaseTestClass.h"
#include "FinancialDetails.h"

#include <gtest/gtest.h>

class FinancialDetailsControllerTest : public BaseTestClass<FinancialDetailsControllerTest, api::v1::FinancialDetails> {
    void setupExpectedValues() override {
        expectedValues["tax_rate"] = 15.00;
        expectedValues["gateway"] = "PayPal";
        expectedValues["gateway_merchant_id"] = "merchant123";
        expectedValues["merchant_id"] = "merchant001";
        expectedValues["merchant_name"] = "Merchant One";
    }

    void setupUpdatedValues() override {
        updatedValues["tax_rate"] = 10.00;
        updatedValues["gateway"] = "Stripe";
        updatedValues["gateway_merchant_id"] = "merchant456";
        updatedValues["merchant_id"] = "merchant002";
        updatedValues["merchant_name"] = "Merchant Two";
    }

    void setupGetOneValues() override {
        getOneValues["tax_rate"] = 15.00;
        getOneValues["gateway"] = "PayPal";
        getOneValues["gateway_merchant_id"] = "merchant123";
        getOneValues["merchant_id"] = "merchant001";
        getOneValues["merchant_name"] = "Merchant One";
    }

    void setupGetListValues() override {
        getListValues["_page"] = 1;
        getListValues["total"] = 2;

        Json::Value data = Json::arrayValue;

        // Entry 1
        Json::Value entry1;
        entry1["gateway"] = "Stripe";
        entry1["gateway_merchant_id"] = "merchant456";
        entry1["merchant_id"] = "merchant002";
        entry1["merchant_name"] = "Merchant Two";
        entry1["tax_rate"] = 10.0;
        entry1["id"] = 2;

        // Entry 2
        Json::Value entry2;
        entry2["gateway"] = "PayPal";
        entry2["gateway_merchant_id"] = "merchant123";
        entry2["merchant_id"] = "merchant001";
        entry2["merchant_name"] = "Merchant One";
        entry2["tax_rate"] = 15.0;
        entry2["id"] = 1;

        data.append(entry1);
        data.append(entry2);

        getListValues["data"] = data;
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

TEST_F(FinancialDetailsControllerTest, GetList200) {
    getList200();
}
