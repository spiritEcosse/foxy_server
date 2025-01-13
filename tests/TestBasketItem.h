#pragma once

#include "BaseTestClass.h"
#include "BasketItem.h"

#include <gtest/gtest.h>

class BasketItemControllerTest : public BaseTestClass<BasketItemControllerTest, api::v1::BasketItem> {
    void setupExpectedValues() override {
        expectedValues["basket_id"] = 3;
        expectedValues["item_id"] = 1;
    }

    void setupUpdatedValues() override {
        updatedValues["basket_id"] = 4;
        updatedValues["item_id"] = 2;
    }

    void setupGetOneValues() override {
        getOneValues["basket_id"] = 1;
        getOneValues["quantity"] = 1;
        getOneValues["item_id"] = 1;
        getOneValues["price"] = 100.00;
        getOneValues["id"] = 1;
    }

    void setupGetListValues() override {
        getListValues["_page"] = 1;
        getListValues["total"] = 2;

        Json::Value data = Json::arrayValue;

        Json::Value item1;
        item1["basket_id"] = 1;
        item1["id"] = 1;
        item1["item_id"] = 1;
        item1["price"] = 100.0;
        item1["quantity"] = 1;

        Json::Value item1_details;
        item1_details["description"] = "Description1";
        item1_details["enabled"] = true;
        item1_details["id"] = 1;
        item1_details["meta_description"] = "Meta1";
        item1_details["price"] = 100.0;
        item1_details["shipping_profile_id"] = 1;
        item1_details["slug"] = "item1";
        item1_details["src"] = "media1.png";
        item1_details["title"] = "Item1";

        item1["item"] = item1_details;

        Json::Value item2;
        item2["basket_id"] = 2;
        item2["id"] = 2;
        item2["item_id"] = 2;
        item2["price"] = 200.0;
        item2["quantity"] = 2;

        Json::Value item2_details;
        item2_details["description"] = "Description2";
        item2_details["enabled"] = false;
        item2_details["id"] = 2;
        item2_details["meta_description"] = "Meta2";
        item2_details["price"] = 200.0;
        item2_details["shipping_profile_id"] = 2;
        item2_details["slug"] = "item2";
        item2_details["src"] = "media2.png";
        item2_details["title"] = "Item2";

        item2["item"] = item2_details;

        data.append(item1);
        data.append(item2);

        getListValues["data"] = data;
    }
};

TEST_F(BasketItemControllerTest, Create200) {
    testCreate200();
}

TEST_F(BasketItemControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(BasketItemControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(BasketItemControllerTest, Delete204) {
    testDelete204();
}

TEST_F(BasketItemControllerTest, Update200) {
    testUpdate200();
}

TEST_F(BasketItemControllerTest, GetOne200) {
    testGetOne200();
}

TEST_F(BasketItemControllerTest, GetList200) {
    testGetList200();
}

TEST_F(BasketItemControllerTest, GetOne404) {
    testGetOne404();
}

TEST_F(BasketItemControllerTest, testDeleteItems) {
    testDeleteItems();
}

TEST_F(BasketItemControllerTest, testCreateItems) {
    testCreateItems();
}

TEST_F(BasketItemControllerTest, testUpdateItems) {
    testUpdateItems();
}
