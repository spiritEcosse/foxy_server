#pragma once

#include "BaseTestClass.h"
#include "Basket.h"

#include <gtest/gtest.h>

class BasketControllerTest : public BaseTestClass<BasketControllerTest, api::v1::Basket> {
    void setupExpectedValues() override {
        expectedValues["user_id"] = 1;
    }

    void setupUpdatedValues() override {
        updatedValues["user_id"] = 2;
    }

    void setupGetOneValues() override {
        getOneValues["user_id"] = 1;
        getOneValues["id"] = 1;
    }
};

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

TEST_F(BasketControllerTest, GetOne200) {
    getOne200();
}
