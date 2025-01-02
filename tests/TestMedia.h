#pragma once

#include "BaseTestClass.h"
#include "Media.h"

#include <gtest/gtest.h>

class MediaControllerTest : public BaseTestClass<MediaControllerTest, api::v1::Media> {
public:
    static constexpr drogon::HttpMethod method = drogon::Post;
    static FieldsMap expectedValues;
};

FieldsMap MediaControllerTest::expectedValues = {
    {"src", "mock_src"},
    {"item_id", 1},
    {"sort", 1},
    {"content_type", "video/mp4"},
    {"type", "video"},
};

TEST_F(MediaControllerTest, Create200) {
    testCreate200();
}

TEST_F(MediaControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(MediaControllerTest, RequiredFields400) {
    testRequiredFields400();
}
