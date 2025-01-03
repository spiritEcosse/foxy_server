#pragma once

#include "BaseTestClass.h"
#include "Basket.h"

#include <gtest/gtest.h>

class BasketControllerTest : public BaseTestClass<BasketControllerTest, api::v1::Basket> {
public:
    static FieldsMap expectedValues;
    static FieldsMap updatedValues;
    static FieldsMap getOneValues;
};

FieldsMap BasketControllerTest::expectedValues = {
    {"user_id", 1},
};

FieldsMap BasketControllerTest::updatedValues = {};
FieldsMap BasketControllerTest::getOneValues = {};

TEST_F(BasketControllerTest, Create200) {
    testCreate200();
}

TEST_F(BasketControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(BasketControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(BasketControllerTest, Delete204) {
    testDelete204();
}

TEST_F(BasketControllerTest, Update200) {
    testUpdate200();
}
