#pragma once

#include "BaseTestClass.h"
#include "SocialMedia.h"

#include <gtest/gtest.h>

class SocialMediaControllerTest : public BaseTestClass<SocialMediaControllerTest, api::v1::SocialMedia> {
public:
    static FieldsMap expectedValues;
    static FieldsMap updatedValues;
};

FieldsMap SocialMediaControllerTest::expectedValues = {{"item_id", 1}, {"title", "Pinterest"}};
FieldsMap SocialMediaControllerTest::updatedValues = {};

TEST_F(SocialMediaControllerTest, Create200) {
    testCreate200();
}

TEST_F(SocialMediaControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(SocialMediaControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(SocialMediaControllerTest, Delete204) {
    testDelete204();
}

TEST_F(SocialMediaControllerTest, Update200) {
    testUpdate200();
}