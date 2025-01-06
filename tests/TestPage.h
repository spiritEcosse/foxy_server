#pragma once

#include "BaseTestClass.h"
#include "Page.h"

#include <gtest/gtest.h>

class PageControllerTest : public BaseTestClass<PageControllerTest, api::v1::Page> {
    void setupExpectedValues() override {
        expectedValues["description"] = "mock description";
        expectedValues["meta_description"] = "mock meta description";
        expectedValues["canonical_url"] = "mock canonical url";
        expectedValues["slug"] = "mock-slug";
        expectedValues["title"] = "mock title";
        expectedValues["enabled"] = true;
    }

    void setupUpdatedValues() override {
        updatedValues["description"] = "new mock description";
        updatedValues["meta_description"] = "new mock meta description";
        updatedValues["canonical_url"] = "new mock canonical url";
        updatedValues["slug"] = "new-mock-slug";
        updatedValues["title"] = "new mock title";
        updatedValues["enabled"] = false;
    }

    void setupGetOneValues() override {
        getOneValues["description"] = "Description1";
        getOneValues["meta_description"] = "Meta1";
        getOneValues["slug"] = "page1";
        getOneValues["title"] = "Page1";
        getOneValues["enabled"] = true;
        getOneValues["id"] = 1;
        getOneValues["canonical_url"] = "www.example.com/page1";
    }

    void setupGetListValues() override {
        getListValues["_page"] = 1;
        getListValues["total"] = 2;
        Json::Value data = Json::arrayValue;

        // TODO: must not be here since it disabled
        {
            Json::Value item;
            item["description"] = "Description2";
            item["enabled"] = false;
            item["id"] = 2;
            item["meta_description"] = "Meta2";
            item["slug"] = "page2";
            item["title"] = "Page2";
            item["canonical_url"] = "www.example.com/page2";  // Example value

            data.append(item);
        }

        {
            Json::Value item;
            item["description"] = "Description1";
            item["enabled"] = true;
            item["id"] = 1;
            item["meta_description"] = "Meta1";
            item["slug"] = "page1";
            item["canonical_url"] = "www.example.com/page1";  // Example value
            item["title"] = "Page1";

            data.append(item);
        }

        getListValues["data"] = data;
    }
};

TEST_F(PageControllerTest, Create200) {
    testCreate200();
}

TEST_F(PageControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(PageControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(PageControllerTest, Delete204) {
    testDelete204();
}

TEST_F(PageControllerTest, Update200) {
    testUpdate200();
}

TEST_F(PageControllerTest, GetOne200) {
    getOne200();
}

TEST_F(PageControllerTest, GetList200) {
    getList200();
}
