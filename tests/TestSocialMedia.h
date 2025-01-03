#pragma once

#include "BaseTestClass.h"
#include "SocialMedia.h"

#include <gtest/gtest.h>

class SocialMediaControllerTest : public BaseTestClass<SocialMediaControllerTest, api::v1::SocialMedia> {
public:
    static constexpr drogon::HttpMethod method = drogon::Post;
    static FieldsMap expectedValues;
};

FieldsMap SocialMediaControllerTest::expectedValues = {{"item_id", 1}, {"title", "Pinterest"}};

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
