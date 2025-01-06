#pragma once

#include "BaseTestClass.h"
#include "Media.h"

#include <gtest/gtest.h>

class MediaControllerTest : public BaseTestClass<MediaControllerTest, api::v1::Media> {
    void setupExpectedValues() override {
        expectedValues["src"] = "mock_src";
        expectedValues["item_id"] = 1;
        expectedValues["sort"] = 1;
        expectedValues["content_type"] = "video/mp4";
        expectedValues["type"] = "video";
    }

    void setupUpdatedValues() override {
        updatedValues["src"] = "new mock_src";
        updatedValues["item_id"] = 2;
        updatedValues["sort"] = 2;
        updatedValues["content_type"] = "image/png";
        updatedValues["type"] = "image";
    }

    void setupGetOneValues() override {
        getOneValues["content_type"] = "image/png";
        getOneValues["id"] = 1;
        getOneValues["item_id"] = 1;
        getOneValues["sort"] = 1;
        getOneValues["src"] = "media1.png";
        getOneValues["type"] = "image";
    }

    void setupGetListValues() override {
        getListValues["_page"] = 1;
        getListValues["total"] = 2;

        Json::Value data = Json::arrayValue;

        Json::Value entry1;
        entry1["content_type"] = "image/png";
        entry1["id"] = 2;
        entry1["item_id"] = 2;
        entry1["sort"] = 2;
        entry1["src"] = "media2.png";
        entry1["type"] = "image";

        Json::Value entry2;
        entry2["content_type"] = "image/png";
        entry2["id"] = 1;
        entry2["item_id"] = 1;
        entry2["sort"] = 1;
        entry2["src"] = "media1.png";
        entry2["type"] = "image";

        data.append(entry1);
        data.append(entry2);

        getListValues["data"] = data;
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

TEST_F(MediaControllerTest, GetOne200) {
    getOne200();
}

TEST_F(MediaControllerTest, GetList200) {
    getList200();
}
