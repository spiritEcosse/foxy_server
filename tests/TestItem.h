#pragma once

#include "BaseTestClass.h"
#include "Item.h"

#include <gtest/gtest.h>

class ItemControllerTest : public BaseTestClass<ItemControllerTest, api::v1::Item> {
    void setupExpectedValues() override {
        expectedValues["description"] = "mock description";
        expectedValues["meta_description"] = "mock meta description";
        expectedValues["price"] = 100.0;
        expectedValues["shipping_profile_id"] = 1;
        expectedValues["slug"] = "mock-slug";
        expectedValues["title"] = "mock title";
        expectedValues["enabled"] = true;
    }

    void setupUpdatedValues() override {
        updatedValues["description"] = "new mock description";
        updatedValues["meta_description"] = "new mock meta description";
        updatedValues["price"] = 200.0;
        updatedValues["shipping_profile_id"] = 2;
        updatedValues["slug"] = "new-mock-slug";
        updatedValues["title"] = "new mock title";
        updatedValues["enabled"] = false;
    }

    void setupGetOneValues() override {
        getOneValues["description"] = "Description1";
        getOneValues["meta_description"] = "Meta1";
        getOneValues["slug"] = "item1";
        getOneValues["title"] = "Item1";
        getOneValues["enabled"] = true;
        getOneValues["id"] = 1;
        getOneValues["shipping_profile_id"] = 1;
        getOneValues["price"] = 100.0;

        // Set up the "media" array
        Json::Value media = Json::arrayValue;
        Json::Value mediaEntry;
        mediaEntry["content_type"] = "image/png";
        mediaEntry["id"] = 1;
        mediaEntry["item_id"] = 1;
        mediaEntry["sort"] = 1;
        mediaEntry["src"] = "media1.png";
        mediaEntry["type"] = "image";

        media.append(mediaEntry);
        getOneValues["media"] = media;
    }

    void setupGetListValues() override {
        Json::Value data = Json::arrayValue;

        Json::Value item;
        item["description"] = "Description1";
        item["enabled"] = true;
        item["id"] = 1;
        item["meta_description"] = "Meta1";
        item["price"] = 100.0;
        item["shipping_profile_id"] = 1;
        item["slug"] = "item1";
        item["src"] = "media1.png";
        item["src_video"] = Json::nullValue;
        item["title"] = "Item1";

        data.append(item);

        getListValues["_page"] = 1;
        getListValues["total"] = 1;
        getListValues["data"] = data;
    }
};

TEST_F(ItemControllerTest, Create200) {

    testCreate200();
}

TEST_F(ItemControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(ItemControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(ItemControllerTest, Delete204) {
    testDelete204();
}

TEST_F(ItemControllerTest, Update200) {
    testUpdate200();
}

TEST_F(ItemControllerTest, GetOne200) {
    getOne200();
}

TEST_F(ItemControllerTest, GetList200) {
    getList200();
}

TEST_F(ItemControllerTest, GetOne404) {
    getOne404();
}

TEST_F(ItemControllerTest, GetOneAdmin200) {
    getOneAdmin200();
}
