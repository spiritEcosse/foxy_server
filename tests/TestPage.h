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
