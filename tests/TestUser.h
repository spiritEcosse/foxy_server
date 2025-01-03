#pragma once

#include "BaseTestClass.h"
#include "User.h"

#include <gtest/gtest.h>

class UserControllerTest : public BaseTestClass<UserControllerTest, api::v1::User> {
    void SetUp() override {
        expectedValues["email"] = "mock email";
        expectedValues["first_name"] = "mock first name";
        expectedValues["last_name"] = "mock last name";
        expectedValues["birthday"] = "2024-01-14";

        updatedValues["email"] = "new mock email";
        updatedValues["first_name"] = "new mock first name";
        updatedValues["last_name"] = "new mock last name";
        updatedValues["birthday"] = "2024-01-14";
        BaseTestClass::SetUp();
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
