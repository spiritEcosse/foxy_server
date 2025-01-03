#pragma once

#include "BaseTestClass.h"
#include "Media.h"

#include <gtest/gtest.h>

class MediaControllerTest : public BaseTestClass<MediaControllerTest, api::v1::Media> {
public:
    static FieldsMap expectedValues;
    static FieldsMap updatedValues;
    static FieldsMap getOneValues;
};

FieldsMap MediaControllerTest::expectedValues = {
    {"src", "mock_src"},
    {"item_id", 1},
    {"sort", 1},
    {"content_type", "video/mp4"},
    {"type", "video"},
};
FieldsMap MediaControllerTest::updatedValues = {};
FieldsMap MediaControllerTest::getOneValues = {};

TEST_F(MediaControllerTest, Create200) {
    testCreate200();
}

TEST_F(MediaControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(MediaControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(MediaControllerTest, Delete204) {
    testDelete204();
}

TEST_F(MediaControllerTest, Update200) {
    testUpdate200();
}
