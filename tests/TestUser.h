#pragma once

#include "BaseTestClass.h"
#include "User.h"

#include <gtest/gtest.h>
#include <drogon/drogon.h>

class UserControllerTest : public BaseTestClass<UserControllerTest, api::v1::User> {
public:
    static constexpr drogon::HttpMethod method = drogon::Post;
    static FieldsMap expectedValues;
};

FieldsMap UserControllerTest::expectedValues = {
    {"email", "mock email"},
    {"first_name", "mock first name"},
    {"last_name", "mock last name"},
    {"birthday", "2024-01-14"},
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
