#pragma once

#include "BaseTestClass.h"
#include "Item.h"

#include <gtest/gtest.h>
#include <drogon/drogon.h>

class ItemControllerTest : public BaseTestClass<ItemControllerTest, api::v1::Item> {
public:
    static constexpr drogon::HttpMethod method = drogon::Post;
    static FieldsMap expectedValues;
};

FieldsMap ItemControllerTest::expectedValues = {
    {"description", "mock description"},
    {"meta_description", "mock meta description"},
    {"price", 100.0},
    {"shipping_profile_id", 1},
    {"slug", "mock-slug"},
    {"title", "mock title"},
    {"enabled", true},
};

TEST_F(ItemControllerTest, Create200) {
    testCreate200();
}

TEST_F(ItemControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(ItemControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(ItemControllerTest, Delete204) {
    testDelete204();
}
