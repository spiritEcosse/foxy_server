#pragma once

#include "BaseTestClass.h"
#include "Address.h"

#include <gtest/gtest.h>

class AddressControllerTest : public BaseTestClass<AddressControllerTest, api::v1::Address> {
    void setupExpectedValues() override {
        expectedValues["address"] = "mock address";
        expectedValues["country_id"] = 1;
        expectedValues["city"] = "mock city";
        expectedValues["zipcode"] = "mock zipcode";
        expectedValues["user_id"] = 1;
    }

    void setupUpdatedValues() override {
        updatedValues["address"] = "new mock address";
        updatedValues["country_id"] = 2;
        updatedValues["city"] = "new mock city";
        updatedValues["zipcode"] = "new mock zipcode";
        updatedValues["user_id"] = 2;
    }

    void setupGetOneValues() override {
        getOneValues["address"] = "Address1";
        getOneValues["country_id"] = 1;
        getOneValues["city"] = "City1";
        getOneValues["zipcode"] = "12345";
        getOneValues["user_id"] = 1;
        getOneValues["id"] = 1;
    }

    void setupGetListValues() override {
        getListValues["_page"] = 1;
        getListValues["total"] = 2;
        Json::Value data = Json::arrayValue;

        {
            Json::Value item;
            item["address"] = "Address1";
            item["city"] = "City1";
            item["country"]["code"] = "C1";
            item["country"]["id"] = 1;
            item["country"]["title"] = "Country1";
            item["country_id"] = 1;
            item["id"] = 1;
            item["user_id"] = 1;
            item["zipcode"] = "12345";

            data.append(item);
        }

        {
            Json::Value item;
            item["address"] = "Address2";
            item["city"] = "City2";
            item["country"]["code"] = "C2";
            item["country"]["id"] = 2;
            item["country"]["title"] = "Country2";
            item["country_id"] = 2;
            item["id"] = 2;
            item["user_id"] = 2;
            item["zipcode"] = "67890";

            data.append(item);
        }

        getListValues["data"] = data;
    }
};

TEST_F(AddressControllerTest, Create200) {
    testCreate200();
}

TEST_F(AddressControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(AddressControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(AddressControllerTest, Delete204) {
    testDelete204();
}

TEST_F(AddressControllerTest, Update200) {
    testUpdate200();
}

TEST_F(AddressControllerTest, GetOne200) {
    testGetOne200();
}

TEST_F(AddressControllerTest, GetList200) {
    testGetList200();
}

TEST_F(AddressControllerTest, GetOne404) {
    testGetOne404();
}

TEST_F(AddressControllerTest, testDeleteItems) {
    testDeleteItems();
}

TEST_F(AddressControllerTest, testCreateItems) {
    testCreateItems();
}

TEST_F(AddressControllerTest, testUpdateItems) {
    testUpdateItems();
}
