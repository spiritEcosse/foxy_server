#pragma once

#include "BaseTestClass.h"
#include "Media.h"

#include <gtest/gtest.h>

class MediaControllerTest : public BaseTestClass<MediaControllerTest, api::v1::Media> {
    void SetUp() override {
        expectedValues["src"] = "mock_src";
        expectedValues["item_id"] = 1;
        expectedValues["sort"] = 1;
        expectedValues["content_type"] = "video/mp4";
        expectedValues["type"] = "video";

        updatedValues["src"] = "new mock_src";
        updatedValues["item_id"] = 2;
        updatedValues["sort"] = 2;
        updatedValues["content_type"] = "image/png";
        updatedValues["type"] = "image";

        BaseTestClass::SetUp();
    }
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

TEST_F(MediaControllerTest, Delete204) {
    testDelete204();
}

TEST_F(MediaControllerTest, Update200) {
    testUpdate200();
}
