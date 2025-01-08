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

    void setupGetListValues() override {
        getListValues["_page"] = 1;
        getListValues["total"] = 4;

        Json::Value data = Json::arrayValue;

        Json::Value item1;
        item1["id"] = 4;
        item1["in_use"] = true;
        item1["user_id"] = 2;

        Json::Value item2;
        item2["id"] = 3;
        item2["in_use"] = true;
        item2["user_id"] = 2;

        Json::Value item3;
        item3["id"] = 2;
        item3["in_use"] = true;
        item3["user_id"] = 2;

        Json::Value item4;
        item4["id"] = 1;
        item4["in_use"] = true;
        item4["user_id"] = 1;

        data.append(item1);
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

TEST_F(BasketControllerTest, GetList200) {
    getList200();
}

TEST_F(BasketControllerTest, GetOne404) {
    getOne404();
}
