#pragma once

#include "BaseTestClass.h"
#include "Review.h"

#include <gtest/gtest.h>

class ReviewControllerTest : public BaseTestClass<ReviewControllerTest, api::v1::Review> {
public:
    static FieldsMap expectedValues;
    static FieldsMap updatedValues;
};

FieldsMap ReviewControllerTest::expectedValues = {
    {"status", "failed"},
    {"user_id", 1},
    {"comment", "mock comment"},
    {"item_id", 1},
};
FieldsMap ReviewControllerTest::updatedValues = {};

TEST_F(ReviewControllerTest, Create200) {
    testCreate200();
}

TEST_F(ReviewControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(ReviewControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(ReviewControllerTest, Delete204) {
    testDelete204();
}

TEST_F(ReviewControllerTest, Update200) {
    testUpdate200();
}
