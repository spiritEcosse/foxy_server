#pragma once

#include "BaseTestClass.h"
#include "BasketItem.h"

#include <gtest/gtest.h>

class BasketItemControllerTest : public BaseTestClass<BasketItemControllerTest, api::v1::BasketItem> {
public:
    static constexpr drogon::HttpMethod method = drogon::Post;
    static FieldsMap expectedValues;
};

FieldsMap BasketItemControllerTest::expectedValues = {
    {"basket_id", 3},
    {"item_id", 1},
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