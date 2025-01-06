#pragma once

#include "BaseTestClass.h"
#include "Review.h"

#include <gtest/gtest.h>

class ReviewControllerTest : public BaseTestClass<ReviewControllerTest, api::v1::Review> {
    void setupExpectedValues() override {
        expectedValues["status"] = "failed";
        expectedValues["user_id"] = 1;
        expectedValues["comment"] = "mock comment";
        expectedValues["item_id"] = 1;
    }

    void setupUpdatedValues() override {
        updatedValues["status"] = "success";
        updatedValues["user_id"] = 2;
        updatedValues["comment"] = "new mock comment";
        updatedValues["item_id"] = 2;
    }

    void setupGetOneValues() override {
        getOneValues["comment"] = "Great product!";
        getOneValues["id"] = 1;
        getOneValues["user_id"] = 1;
        getOneValues["item_id"] = 1;
        getOneValues["status"] = "Approved";
    }

    void setupGetListValues() override {
        getListValues["_page"] = 1;
        getListValues["total"] = 2;

        Json::Value data = Json::arrayValue;

        Json::Value entry1;
        entry1["comment"] = "Good quality.";
        entry1["id"] = 2;
        entry1["item_id"] = 2;
        entry1["status"] = "Pending";
        entry1["user_id"] = 2;

        Json::Value entry2;
        entry2["comment"] = "Great product!";
        entry2["id"] = 1;
        entry2["item_id"] = 1;
        entry2["status"] = "Approved";
        entry2["user_id"] = 1;

        data.append(entry1);
        data.append(entry2);

        getListValues["data"] = data;
    }
};

TEST_F(ReviewControllerTest, Create200) {
    testCreate200();
}

TEST_F(ReviewControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(ReviewControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(ReviewControllerTest, Delete204) {
    testDelete204();
}

TEST_F(ReviewControllerTest, Update200) {
    testUpdate200();
}

TEST_F(ReviewControllerTest, GetOne200) {
    getOne200();
}

TEST_F(ReviewControllerTest, GetList200) {
    getList200();
}
