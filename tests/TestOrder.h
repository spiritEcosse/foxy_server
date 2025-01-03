#pragma once

#include "BaseTestClass.h"
#include "Order.h"

#include <gtest/gtest.h>

class OrderControllerTest : public BaseTestClass<OrderControllerTest, api::v1::Order> {
public:
    static FieldsMap expectedValues;
    static FieldsMap updatedValues;
    static FieldsMap getOneValues;
};

FieldsMap OrderControllerTest::expectedValues = {
    {"status", "Ordered"},
    {"basket_id", 3},
    {"total", 1.0},
    {"total_ex_taxes", 1.0},
    {"tax_rate", 1.0},
    {"taxes", 1.0},
    {"user_id", 1},
    {"address_id", 1},
};
FieldsMap OrderControllerTest::updatedValues = {};
FieldsMap OrderControllerTest::getOneValues = {};

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
