#pragma once

#include "BaseTestClass.h"
#include "Country.h"

#include <gtest/gtest.h>

class CountryControllerTest : public BaseTestClass<CountryControllerTest, api::v1::Country> {
    void setupExpectedValues() override {
        expectedValues["title"] = "mock title";
        expectedValues["code"] = "mock code";
    }

    void setupUpdatedValues() override {
        updatedValues["title"] = "new mock title";
        updatedValues["code"] = "new mock code";
    }

    void setupGetOneValues() override {
        getOneValues["title"] = "Country1";
        getOneValues["code"] = "C1";
        getOneValues["id"] = 1;
    }

    void setupGetListValues() override {
        getListValues["_page"] = 1;
        getListValues["total"] = 2;

        Json::Value data = Json::arrayValue;

        Json::Value entry1;
        entry1["code"] = "C2";
        entry1["id"] = 2;
        entry1["title"] = "Country2";

        Json::Value entry2;
        entry2["code"] = "C1";
        entry2["id"] = 1;
        entry2["title"] = "Country1";

        data.append(entry1);
        data.append(entry2);

        getListValues["data"] = data;
    }
};

TEST_F(CountryControllerTest, Create200) {
    testCreate200();
}

TEST_F(CountryControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(CountryControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(CountryControllerTest, Delete204) {
    testDelete204();
}

TEST_F(CountryControllerTest, Update200) {
    testUpdate200();
}

TEST_F(CountryControllerTest, GetOne200) {
    getOne200();
}

TEST_F(CountryControllerTest, GetList200) {
    getList200();
}
