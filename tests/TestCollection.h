#pragma once

#include "BaseTestClass.h"
#include "controllers/Collection.h"

#include <gtest/gtest.h>

class CollectionControllerTest : public BaseTestClass<CollectionControllerTest, api::v1::Collection> {
    void setupExpectedValues() override {
        expectedValues["title"] = "mock title";
        expectedValues["slug"] = "mock-slug";
        expectedValues["description"] = "mock description";
        expectedValues["meta_description"] = "mock meta description";
        expectedValues["enabled"] = true;
    }

    void setupUpdatedValues() override {
        updatedValues["title"] = "new mock title";
        updatedValues["slug"] = "new-mock-slug";
        updatedValues["description"] = "new mock description";
        updatedValues["meta_description"] = "new mock meta description";
        updatedValues["enabled"] = false;
    }

    void setupGetOneValues() override {
        getOneValues["id"] = 1;
        getOneValues["title"] = "Collection1";
        getOneValues["slug"] = "collection1";
        getOneValues["description"] = "Collection description 1";
        getOneValues["meta_description"] = "Collection meta 1";
        getOneValues["enabled"] = true;

        // collection1 holds item 1 and item 2 (ordered by item.id ASC)
        Json::Value items = Json::arrayValue;

        Json::Value item1;
        item1["id"] = 1;
        item1["title"] = "Item1";
        item1["slug"] = "item1";
        item1["description"] = "Description1";
        item1["meta_description"] = "Meta1";
        item1["price"] = 100.0;
        item1["shipping_profile_id"] = 1;
        item1["enabled"] = true;

        Json::Value item2;
        item2["id"] = 2;
        item2["title"] = "Item2";
        item2["slug"] = "item2";
        item2["description"] = "Description2";
        item2["meta_description"] = "Meta2";
        item2["price"] = 200.0;
        item2["shipping_profile_id"] = 2;
        item2["enabled"] = false;

        items.append(item1);
        items.append(item2);
        getOneValues["items"] = items;
    }

    void setupGetListValues() override {
        Json::Value data = Json::arrayValue;

        // Only enabled collections are listed publicly (Collection2 is disabled)
        Json::Value collection;
        collection["id"] = 1;
        collection["title"] = "Collection1";
        collection["slug"] = "collection1";
        collection["description"] = "Collection description 1";
        collection["meta_description"] = "Collection meta 1";
        collection["enabled"] = true;

        data.append(collection);

        getListValues["_page"] = 1;
        getListValues["total"] = 1;
        getListValues["data"] = data;
    }

public:
    // Public getOne is slug-only: invoke with the slug, not a numeric id.
    void testGetOneBySlug200() {
        setupGetOneValues();
        runAsyncTest<void>([this](auto promise) {
            drogon::app().getLoop()->queueInLoop([this, promise]() {
                controller.getOne(*reqPtr, getOneCallback(promise), "collection1");
            });
        }).get();
    }

    // A numeric id must NOT resolve publicly — it returns 404.
    void testGetOneByNumericId404() {
        runAsyncTest<void>([this](auto promise) {
            drogon::app().getLoop()->queueInLoop([this, promise]() {
                controller.getOne(*reqPtr, getOneCallback(promise, drogon::k404NotFound), "1");
            });
        }).get();
    }
};

TEST_F(CollectionControllerTest, Create200) {
    testCreate200();
}

TEST_F(CollectionControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(CollectionControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(CollectionControllerTest, Delete204) {
    testDelete204();
}

TEST_F(CollectionControllerTest, Update200) {
    testUpdate200();
}

TEST_F(CollectionControllerTest, GetOneBySlug200) {
    testGetOneBySlug200();
}

TEST_F(CollectionControllerTest, GetOneByNumericId404) {
    testGetOneByNumericId404();
}

TEST_F(CollectionControllerTest, GetList200) {
    testGetList200();
}
