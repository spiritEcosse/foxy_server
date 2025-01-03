#pragma once

#include "BaseTestClass.h"
#include "Order.h"

#include <gtest/gtest.h>

class OrderControllerTest : public BaseTestClass<OrderControllerTest, api::v1::Order> {
    void SetUp() override {
        expectedValues["status"] = "Ordered";
        expectedValues["basket_id"] = 3;
        expectedValues["total"] = 1.0;
        expectedValues["total_ex_taxes"] = 1.0;
        expectedValues["tax_rate"] = 1.0;
        expectedValues["taxes"] = 1.0;
        expectedValues["user_id"] = 1;
        expectedValues["address_id"] = 1;

        updatedValues["status"] = "Shipped";
        updatedValues["basket_id"] = 4;
        updatedValues["total"] = 2.0;
        updatedValues["total_ex_taxes"] = 2.0;
        updatedValues["tax_rate"] = 2.0;
        updatedValues["taxes"] = 2.0;
        updatedValues["user_id"] = 2;
        updatedValues["address_id"] = 2;

        BaseTestClass::SetUp();
    }
};

TEST_F(OrderControllerTest, Create200) {
    testCreate200();
}

TEST_F(OrderControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(OrderControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(OrderControllerTest, Delete204) {
    testDelete204();
}

TEST_F(OrderControllerTest, Update200) {
    testUpdate200();
}
