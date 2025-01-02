#pragma once

#include "BaseTestClass.h"
#include "Basket.h"

#include <gtest/gtest.h>
#include <drogon/drogon.h>

class BasketControllerTest : public BaseTestClass<BasketControllerTest, api::v1::Basket> {
public:
    static constexpr drogon::HttpMethod method = drogon::Post;
    static FieldsMap expectedValues;
};

FieldsMap BasketControllerTest::expectedValues = {
         {"user_id", 1},
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
