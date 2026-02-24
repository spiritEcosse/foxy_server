#pragma once

#include "BaseTestClass.h"
#include "controllers/SocialMedia.h"

#include <gtest/gtest.h>

class SocialMediaControllerTest : public BaseTestClass<SocialMediaControllerTest, api::v1::SocialMedia> {
    void setupExpectedValues() override {
        expectedValues["item_id"] = 1;
        expectedValues["title"] = "Pinterest";
        expectedValues["external_id"] = "3453453654675rtydfghr645";
    }

    void setupUpdatedValues() override {
        updatedValues["title"] = "Twitter";
        updatedValues["item_id"] = 2;
        updatedValues["external_id"] = "jfwo48ru8y587yer87234";
    }

    void setupGetOneValues() override {
        getOneValues["external_id"] = "100";
        getOneValues["id"] = 1;
        getOneValues["item_id"] = 1;
        getOneValues["social_url"] = "https://www.youtube.com/watch?v=100";
        getOneValues["title"] = "YouTube";
    }

    void setupGetListValues() override {
        getListValues["_page"] = 1;
        getListValues["total"] = 4;

        Json::Value data = Json::arrayValue;

        Json::Value entry1;
        entry1["external_id"] = "100";
        entry1["id"] = 1;
        entry1["item_id"] = 1;
        entry1["social_url"] = "https://www.youtube.com/watch?v=100";
        entry1["title"] = "YouTube";

        Json::Value entry2;
        entry2["external_id"] = "20000";
        entry2["id"] = 2;
        entry2["item_id"] = 2;
        entry2["social_url"] = "https://x.com/faithfishart/status/20000";
        entry2["title"] = "Twitter";

        Json::Value entry3;
        entry3["external_id"] = "30000";
        entry3["id"] = 3;
        entry3["item_id"] = 1;
        entry3["social_url"] = "https://pinterest.com/pin/30000";
        entry3["title"] = "Pinterest";

        Json::Value entry4;
        entry4["external_id"] = "40000";
        entry4["id"] = 4;
        entry4["item_id"] = 1;
        entry4["social_url"] = "https://x.com/faithfishart/status/40000";
        entry4["title"] = "Twitter";

        data.append(entry1);
        data.append(entry2);
        data.append(entry3);
        data.append(entry4);

        getListValues["data"] = data;
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
    testGetOne200();
}

TEST_F(SocialMediaControllerTest, GetList200) {
    testGetList200();
}

TEST_F(SocialMediaControllerTest, GetOne404) {
    testGetOne404();
}

TEST_F(SocialMediaControllerTest, testDeleteItems) {
    testDeleteItems();
}

TEST_F(SocialMediaControllerTest, testCreateItems) {
    testCreateItems();
}

TEST_F(SocialMediaControllerTest, testUpdateItems) {
    testUpdateItems();
}
