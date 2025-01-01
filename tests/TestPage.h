#pragma once

#include "BaseTestClass.h"
#include "Page.h"

#include <gtest/gtest.h>
#include <drogon/drogon.h>

class PageControllerTest : public BaseTestClass<PageControllerTest, api::v1::Page> {
public:
    static constexpr drogon::HttpMethod method = drogon::Post;
    static FieldsMap expectedValues;
};

FieldsMap PageControllerTest::expectedValues =  {
        {"description", "mock description"},
        {"meta_description", "mock meta description"},
        {"canonical_url", "mock canonical url"},
        {"slug", "mock-slug"},
        {"title", "mock title"},
        {"enabled", true},
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
