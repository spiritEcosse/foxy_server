#pragma once

#include "BaseTestClass.h"
#include "Order.h"

#include <gtest/gtest.h>

class OrderControllerTest : public BaseTestClass<OrderControllerTest, api::v1::Order> {
    void setupExpectedValues() override {
        expectedValues["status"] = "Ordered";
        expectedValues["basket_id"] = 3;
        expectedValues["total"] = 1.0;
        expectedValues["total_ex_taxes"] = 1.0;
        expectedValues["tax_rate"] = 1.0;
        expectedValues["taxes"] = 1.0;
        expectedValues["user_id"] = 1;
        expectedValues["address_id"] = 1;
    }

    void setupUpdatedValues() override {
        updatedValues["status"] = "Shipped";
        updatedValues["basket_id"] = 4;
        updatedValues["total"] = 2.0;
        updatedValues["total_ex_taxes"] = 2.0;
        updatedValues["tax_rate"] = 2.0;
        updatedValues["taxes"] = 2.0;
        updatedValues["user_id"] = 2;
        updatedValues["address_id"] = 2;
    }

    void setupGetOneValues() override {
        // Adding the order details with items
        getOneValues["address_id"] = 1;
        getOneValues["basket_id"] = 1;
        getOneValues["id"] = 1;
        getOneValues["returned"] = false;
        getOneValues["status"] = "Completed";
        getOneValues["tax_rate"] = 0.10000000000000001;
        getOneValues["taxes"] = 10.0;
        getOneValues["total"] = 100.0;
        getOneValues["total_ex_taxes"] = 90.0;
        getOneValues["user_id"] = 1;

        Json::Value items = Json::arrayValue;
        Json::Value item;
        item["description"] = "Description1";
        item["enabled"] = true;
        item["id"] = 1;
        item["meta_description"] = "Meta1";
        item["price"] = 100.0;
        item["shipping_profile_id"] = 1;
        item["slug"] = "item1";
        item["title"] = "Item1";
        items.append(item);  // Add the item to the array

        // Add the items array to the main getOneValues
        getOneValues["items"] = items;
    }
};

TEST_F(OrderControllerTest, Create200) {
    testCreate200();
}

TEST_F(OrderControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(OrderControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(OrderControllerTest, Delete204) {
    testDelete204();
}

TEST_F(OrderControllerTest, Update200) {
    testUpdate200();
}

TEST_F(OrderControllerTest, GetOne200) {
    getOne200();
}
