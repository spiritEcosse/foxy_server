#pragma once

#include "BaseTestClass.h"
#include "controllers/CollectionItem.h"

#include <gtest/gtest.h>

class CollectionItemControllerTest : public BaseTestClass<CollectionItemControllerTest, api::v1::CollectionItem> {
    void setupExpectedValues() override {
        expectedValues["collection_id"] = 2;
        expectedValues["item_id"] = 2;
    }

    void setupUpdatedValues() override {
        // Must not collide with any existing (collection_id, item_id) pair in fixtures.
        updatedValues["collection_id"] = 2;
        updatedValues["item_id"] = 2;
    }

    void setupGetOneValues() override {
        getOneValues["id"] = 1;
        getOneValues["collection_id"] = 1;
        getOneValues["item_id"] = 1;
    }

    void setupGetListValues() override {
        getListValues["_page"] = 1;
        getListValues["total"] = 3;

        Json::Value data = Json::arrayValue;

        // Default list ordering is updated_at DESC, id DESC; all three rows share
        // the same timestamp, so the id tiebreak yields 3, 2, 1.
        Json::Value link3;
        link3["id"] = 3;
        link3["collection_id"] = 2;
        link3["item_id"] = 1;

        Json::Value link2;
        link2["id"] = 2;
        link2["collection_id"] = 1;
        link2["item_id"] = 2;

        Json::Value link1;
        link1["id"] = 1;
        link1["collection_id"] = 1;
        link1["item_id"] = 1;

        data.append(link3);
        data.append(link2);
        data.append(link1);

        getListValues["data"] = data;
    }
};

TEST_F(CollectionItemControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(CollectionItemControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(CollectionItemControllerTest, GetList200) {
    testGetList200();
}

TEST_F(CollectionItemControllerTest, testCreateItems) {
    testCreateItems();
}

TEST_F(CollectionItemControllerTest, testUpdateItems) {
    testUpdateItems();
}

TEST_F(CollectionItemControllerTest, testDeleteItems) {
    testDeleteItems();
}
