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

    void setupGetListValues() override {
        getListValues["_page"] = 1;
        getListValues["total"] = 2;

        Json::Value data = Json::arrayValue;

        Json::Value entry1;
        entry1["external_id"] = "100";
        entry1["id"] = 1;
        entry1["item_id"] = 1;
        entry1["social_url"] = "100";
        entry1["title"] = "Facebook";

        Json::Value entry2;
        entry2["external_id"] = "20000";
        entry2["id"] = 2;
        entry2["item_id"] = 2;
        entry2["social_url"] = "https://x.com/faithfishart/status/20000";
        entry2["title"] = "Twitter";

        data.append(entry1);
        data.append(entry2);

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
    getOne200();
}

TEST_F(SocialMediaControllerTest, GetList200) {
    getList200();
}
