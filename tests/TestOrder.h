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

    void setupGetListValues() override {
        getListValues["_page"] = 1;
        getListValues["total"] = 2;
        Json::Value data = Json::arrayValue;

        {
            Json::Value item;
            item["address"]["address"] = "Address2";
            item["address"]["city"] = "City2";
            item["address"]["country"]["code"] = "C2";
            item["address"]["country"]["id"] = 2;
            item["address"]["country"]["title"] = "Country2";
            item["address"]["country_id"] = 2;
            item["address"]["id"] = 2;
            item["address"]["user_id"] = 2;
            item["address"]["zipcode"] = "67890";
            item["address_id"] = 2;
            item["basket_id"] = 2;

            Json::Value basketItem;
            basketItem["basket_id"] = 2;
            basketItem["id"] = 2;
            basketItem["item"]["description"] = "Description2";
            basketItem["item"]["enabled"] = false;
            basketItem["item"]["id"] = 2;
            basketItem["item"]["meta_description"] = "Meta2";
            basketItem["item"]["price"] = 200.0;
            basketItem["item"]["shipping_profile_id"] = 2;
            basketItem["item"]["slug"] = "item2";
            basketItem["item"]["src"] = "media2.png";
            basketItem["item"]["title"] = "Item2";
            basketItem["item_id"] = 2;
            basketItem["price"] = 200.0;
            basketItem["quantity"] = 2;

            item["basket_items"].append(basketItem);
            item["id"] = 2;
            item["returned"] = false;
            item["status"] = "Processing";
            item["tax_rate"] = 0.1;
            item["taxes"] = 20.0;
            item["total"] = 200.0;
            item["total_ex_taxes"] = 180.0;
            item["user"]["first_name"] = "User";
            item["user"]["id"] = 2;
            item["user"]["last_name"] = "Two";
            item["user_id"] = 2;

            data.append(item);
        }

        {
            Json::Value item;
            item["address"]["address"] = "Address1";
            item["address"]["city"] = "City1";
            item["address"]["country"]["code"] = "C1";
            item["address"]["country"]["id"] = 1;
            item["address"]["country"]["title"] = "Country1";
            item["address"]["country_id"] = 1;
            item["address"]["id"] = 1;
            item["address"]["user_id"] = 1;
            item["address"]["zipcode"] = "12345";
            item["address_id"] = 1;
            item["basket_id"] = 1;

            Json::Value basketItem;
            basketItem["basket_id"] = 1;
            basketItem["id"] = 1;
            basketItem["item"]["description"] = "Description1";
            basketItem["item"]["enabled"] = true;
            basketItem["item"]["id"] = 1;
            basketItem["item"]["meta_description"] = "Meta1";
            basketItem["item"]["price"] = 100.0;
            basketItem["item"]["shipping_profile_id"] = 1;
            basketItem["item"]["slug"] = "item1";
            basketItem["item"]["src"] = "media1.png";
            basketItem["item"]["title"] = "Item1";
            basketItem["item_id"] = 1;
            basketItem["price"] = 100.0;
            basketItem["quantity"] = 1;

            item["basket_items"].append(basketItem);
            item["id"] = 1;
            item["returned"] = false;
            item["status"] = "Completed";
            item["tax_rate"] = 0.1;
            item["taxes"] = 10.0;
            item["total"] = 100.0;
            item["total_ex_taxes"] = 90.0;
            item["user"]["first_name"] = "User";
            item["user"]["id"] = 1;
            item["user"]["last_name"] = "One";
            item["user_id"] = 1;

            data.append(item);
        }

        getListValues["data"] = data;
    }

    void setupOrderDetails() {
        // Setup address and country
        Json::Value country;
        country["code"] = "C1";
        country["id"] = 1;
        country["title"] = "Country1";

        Json::Value address;
        address["address"] = "Address1";
        address["city"] = "City1";
        address["country"] = country;
        address["country_id"] = 1;
        address["id"] = 1;
        address["user_id"] = 1;
        address["zipcode"] = "12345";

        getOneAdminValues["_address"] = address;

        // Setup items array
        Json::Value item;
        item["id"] = 1;
        item["price"] = 100.0;
        item["quantity"] = 1;
        item["title"] = "Item1";

        Json::Value items = Json::arrayValue;
        items.append(item);
        getOneAdminValues["items"] = items;

        // Setup order details
        getOneAdminValues["address_id"] = 1;
        getOneAdminValues["basket_id"] = 1;
        getOneAdminValues["id"] = 1;
        getOneAdminValues["returned"] = false;
        getOneAdminValues["status"] = "Completed";
        getOneAdminValues["tax_rate"] = 0.10000000000000001;
        getOneAdminValues["taxes"] = 10.0;
        getOneAdminValues["total"] = 100.0;
        getOneAdminValues["total_ex_taxes"] = 90.0;
        getOneAdminValues["user_id"] = 1;

        // Setup user details
        Json::Value user;
        user["birthday"] = "2000-01-01";
        user["email"] = "user1@example.com";
        user["first_name"] = "User";
        user["has_newsletter"] = true;
        user["id"] = 1;
        user["is_admin"] = false;
        user["last_name"] = "One";

        getOneAdminValues["user"] = user;
    }

    void setupGetOneAdmin() override {
        // Base order fields
        getOneAdminValues["address_id"] = 1;
        getOneAdminValues["basket_id"] = 1;
        getOneAdminValues["id"] = 1;
        getOneAdminValues["returned"] = false;
        getOneAdminValues["status"] = "Completed";
        getOneAdminValues["tax_rate"] = 0.10000000000000001;
        getOneAdminValues["taxes"] = 10.0;
        getOneAdminValues["total"] = 100.0;
        getOneAdminValues["total_ex_taxes"] = 90.0;
        getOneAdminValues["user_id"] = 1;

        // Setup user object
        Json::Value user;
        user["birthday"] = "2000-01-01";
        user["email"] = "user1@example.com";
        user["first_name"] = "User";
        user["has_newsletter"] = true;
        user["id"] = 1;
        user["is_admin"] = false;
        user["last_name"] = "One";
        getOneAdminValues["user"] = user;

        // Setup country object
        Json::Value country;
        country["code"] = "C1";
        country["id"] = 1;
        country["title"] = "Country1";

        // Setup address object with nested country
        Json::Value address;
        address["address"] = "Address1";
        address["city"] = "City1";
        address["country"] = country;
        address["country_id"] = 1;
        address["id"] = 1;
        address["user_id"] = 1;
        address["zipcode"] = "12345";
        getOneAdminValues["address"] = address;

        // Setup item details
        Json::Value itemDetails;
        itemDetails["description"] = "Description1";
        itemDetails["enabled"] = true;
        itemDetails["id"] = 1;
        itemDetails["meta_description"] = "Meta1";
        itemDetails["price"] = 100.0;
        itemDetails["shipping_profile_id"] = 1;
        itemDetails["slug"] = "item1";
        itemDetails["src"] = "media1.png";
        itemDetails["title"] = "Item1";

        // Setup basket item with nested item details
        Json::Value basketItem;
        basketItem["basket_id"] = 1;
        basketItem["id"] = 1;
        basketItem["item"] = itemDetails;
        basketItem["item_id"] = 1;
        basketItem["price"] = 100.0;
        basketItem["quantity"] = 1;

        // Create items array and append basket item
        Json::Value items = Json::arrayValue;
        items.append(basketItem);
        getOneAdminValues["items"] = items;
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

TEST_F(OrderControllerTest, GetList200) {
    getList200();
}

TEST_F(OrderControllerTest, GetOne404) {
    getOne404();
}

TEST_F(OrderControllerTest, GetOneAdmin200) {
    getOneAdmin200();
}
