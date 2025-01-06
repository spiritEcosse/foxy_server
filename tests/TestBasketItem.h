#pragma once

#include "BaseTestClass.h"
#include "BasketItem.h"

#include <gtest/gtest.h>

class BasketItemControllerTest : public BaseTestClass<BasketItemControllerTest, api::v1::BasketItem> {
    void setupExpectedValues() override {
        expectedValues["basket_id"] = 3;
        expectedValues["item_id"] = 1;
    }

    void setupUpdatedValues() override {
        updatedValues["basket_id"] = 4;
        updatedValues["item_id"] = 2;
    }

    void setupGetOneValues() override {
        getOneValues["basket_id"] = 1;
        getOneValues["quantity"] = 1;
        getOneValues["item_id"] = 1;
        getOneValues["price"] = 100.00;
        getOneValues["id"] = 1;
    }
};

TEST_F(BasketItemControllerTest, Create200) {
    testCreate200();
}

TEST_F(BasketItemControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(BasketItemControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(BasketItemControllerTest, Delete204) {
    testDelete204();
}

TEST_F(BasketItemControllerTest, Update200) {
    testUpdate200();
}

TEST_F(BasketItemControllerTest, GetOne200) {
    getOne200();
}
