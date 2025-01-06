#pragma once

#include "BaseTestClass.h"
#include "User.h"

#include <gtest/gtest.h>

class UserControllerTest : public BaseTestClass<UserControllerTest, api::v1::User> {
    void setupExpectedValues() override {
        expectedValues["email"] = "mock email";
        expectedValues["first_name"] = "mock first name";
        expectedValues["last_name"] = "mock last name";
        expectedValues["birthday"] = "2024-01-14";
    }

    void setupUpdatedValues() override {
        updatedValues["email"] = "new mock email";
        updatedValues["first_name"] = "new mock first name";
        updatedValues["last_name"] = "new mock last name";
        updatedValues["birthday"] = "2024-01-14";
    }

    void setupGetOneValues() override {
        getOneValues["birthday"] = "2000-01-01";
        getOneValues["email"] = "user1@example.com";
        getOneValues["first_name"] = "User";
        getOneValues["has_newsletter"] = true;
        getOneValues["id"] = 1;
        getOneValues["is_admin"] = false;
        getOneValues["last_name"] = "One";
    }
};

TEST_F(UserControllerTest, Create200) {
    testCreate200();
}

TEST_F(UserControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(UserControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(UserControllerTest, Delete204) {
    testDelete204();
}

TEST_F(UserControllerTest, Update200) {
    testUpdate200();
}

TEST_F(UserControllerTest, GetOne200) {
    getOne200();
}