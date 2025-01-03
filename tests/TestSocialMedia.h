#pragma once

#include "BaseTestClass.h"
#include "SocialMedia.h"

#include <gtest/gtest.h>

class SocialMediaControllerTest : public BaseTestClass<SocialMediaControllerTest, api::v1::SocialMedia> {
    void SetUp() override {
        expectedValues["item_id"] = 1;
        expectedValues["title"] = "Pinterest";

        updatedValues["title"] = "Twitter";
        updatedValues["item_id"] = 2;

        BaseTestClass::SetUp();
    }
};

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
