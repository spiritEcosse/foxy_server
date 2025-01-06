#pragma once

#include "BaseTestClass.h"
#include "SocialMedia.h"

#include <gtest/gtest.h>

class SocialMediaControllerTest : public BaseTestClass<SocialMediaControllerTest, api::v1::SocialMedia> {
    void setupExpectedValues() override {
        expectedValues["item_id"] = 1;
        expectedValues["title"] = "Pinterest";
    }

    void setupUpdatedValues() override {
        updatedValues["title"] = "Twitter";
        updatedValues["item_id"] = 2;
    }

    void setupGetOneValues() override {
        getOneValues["external_id"] = "100";
        getOneValues["id"] = 1;
        getOneValues["item_id"] = 1;
        getOneValues["social_url"] = "100";
        getOneValues["title"] = "Facebook";
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

TEST_F(SocialMediaControllerTest, GetOne200) {
    getOne200();
}
