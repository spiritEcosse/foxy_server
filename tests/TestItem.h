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

        Json::Value nets;
        Json::Value net1;
        net1["id"] = 1;
        net1["title"] = "YouTube";
        net1["social_url"] = "https://www.youtube.com/watch?v=100";
        nets.append(net1);

        Json::Value net2;
        net2["id"] = 3;
        net2["title"] = "Pinterest";
        net2["social_url"] = "https://pinterest.com/pin/30000";
        nets.append(net2);

        Json::Value net3;
        net3["id"] = 4;
        net3["title"] = "Twitter";
        net3["social_url"] = "https://x.com/faithfishart/status/40000";
        nets.append(net3);
        getOneValues["nets"] = nets;

        // Set up the "media" array
        Json::Value media = Json::arrayValue;
        Json::Value mediaEntry1;
        mediaEntry1["content_type"] = "image/png";
        mediaEntry1["id"] = 1;
        mediaEntry1["item_id"] = 1;
        mediaEntry1["sort"] = 1;
        mediaEntry1["src"] = "media1.png";
        mediaEntry1["type"] = "image";

        Json::Value mediaEntry2;
        mediaEntry2["content_type"] = "video/mp4";
        mediaEntry2["id"] = 3;
        mediaEntry2["item_id"] = 1;
        mediaEntry2["sort"] = 3;
        mediaEntry2["src"] = "media3.mp4";
        mediaEntry2["type"] = "video";

        media.append(mediaEntry1);
        media.append(mediaEntry2);
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
        item["src_video"] = "media3.mp4";
        item["title"] = "Item1";

        data.append(item);

        getListValues["_page"] = 1;
        getListValues["total"] = 1;
        getListValues["data"] = data;
    }

    void setupGetOneAdmin() override {
        getOneAdminValues["description"] = "Description1";
        getOneAdminValues["enabled"] = true;
        getOneAdminValues["id"] = 1;
        getOneAdminValues["meta_description"] = "Meta1";
        getOneAdminValues["price"] = 100.0;
        getOneAdminValues["shipping_profile_id"] = 1;
        getOneAdminValues["slug"] = "item1";
        getOneAdminValues["title"] = "Item1";

        Json::Value tagItem;
        tagItem["id"] = 1;
        tagItem["title"] = "Tag1";
        Json::Value socialMedia = Json::arrayValue;
        socialMedia.append("Facebook");
        tagItem["social_media"] = socialMedia;

        Json::Value tags = Json::arrayValue;
        tags.append(tagItem);
        getOneAdminValues["tags"] = tags;

        Json::Value mediaEntry1;
        mediaEntry1["id"] = 1;
        mediaEntry1["type"] = "image";
        mediaEntry1["src"] = "media1.png";
        mediaEntry1["content_type"] = "image/png";
        mediaEntry1["sort"] = 1;

        Json::Value mediaEntry2;
        mediaEntry2["content_type"] = "video/mp4";
        mediaEntry2["id"] = 3;
        mediaEntry2["sort"] = 3;
        mediaEntry2["src"] = "media3.mp4";
        mediaEntry2["type"] = "video";

        Json::Value media = Json::arrayValue;
        media.append(mediaEntry1);
        media.append(mediaEntry2);
        getOneAdminValues["media"] = media;
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
    testGetOne200();
}

TEST_F(ItemControllerTest, GetList200) {
    testGetList200();
}

TEST_F(ItemControllerTest, GetOne404) {
    testGetOne404();
}

TEST_F(ItemControllerTest, GetOneAdmin200) {
    testGetOneAdmin200();
}

TEST_F(ItemControllerTest, testDeleteItems) {
    testDeleteItems();
}

TEST_F(ItemControllerTest, testCreateItems) {
    testCreateItems();
}

TEST_F(ItemControllerTest, testUpdateItems) {
    testUpdateItems();
}
