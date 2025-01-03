#pragma once

#include "BaseTestClass.h"
#include "Page.h"

#include <gtest/gtest.h>

class PageControllerTest : public BaseTestClass<PageControllerTest, api::v1::Page> {
public:
    static FieldsMap expectedValues;
    static FieldsMap updatedValues;
};

FieldsMap PageControllerTest::expectedValues = {
    {"description", "mock description"},
    {"meta_description", "mock meta description"},
    {"canonical_url", "mock canonical url"},
    {"slug", "mock-slug"},
    {"title", "mock title"},
    {"enabled", true},
};
FieldsMap PageControllerTest::updatedValues = {};

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
