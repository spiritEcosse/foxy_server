#pragma once

#include "BaseTestClass.h"
#include "controllers/PinterestOAuth.h"
#include <gtest/gtest.h>

class PinterestOAuthTest : public BaseTestClass<PinterestOAuthTest, api::v1::PinterestOAuth> {
    void setupExpectedValues() override {}
    void setupUpdatedValues() override {}

    void setupGetOneValues() override {
        getOneValues["id"] = 1;
        getOneValues["access_token"] = "fixture_access_token";
        getOneValues["refresh_token"] = "fixture_refresh_token";
        getOneValues["scope"] = "pins:read,pins:write,user_accounts:read,boards:read,boards:write";
        getOneValues["singleton"] = true;
    }

    void setupGetListValues() override {
        getListValues["_page"] = 1;
        getListValues["total"] = 1;

        Json::Value entry;
        entry["id"] = 1;
        entry["access_token"] = "fixture_access_token";
        entry["refresh_token"] = "fixture_refresh_token";
        entry["scope"] = "pins:read,pins:write,user_accounts:read,boards:read,boards:write";
        entry["singleton"] = true;

        Json::Value data = Json::arrayValue;
        data.append(entry);
        getListValues["data"] = data;
    }

public:
    void testGetOAuthUrl200() {
        runAsyncTest<void>([this](auto promise) {
            drogon::app().getLoop()->queueInLoop([this, promise]() {
                auto req = drogon::HttpRequest::newHttpRequest();
                controller.getOAuthUrl(req, [promise](const drogon::HttpResponsePtr &resp) {
                    try {
                        EXPECT_EQ(resp->getStatusCode(), drogon::k200OK);
                        EXPECT_EQ(resp->contentType(), drogon::CT_APPLICATION_JSON);
                        const auto json = resp->getJsonObject();
                        ASSERT_NE(json, nullptr);
                        ASSERT_TRUE(json->isMember("url"));
                        const std::string url = (*json)["url"].asString();
                        EXPECT_FALSE(url.empty());
                        EXPECT_NE(url.find("oauth"), std::string::npos);
                        EXPECT_NE(url.find("response_type=code"), std::string::npos);
                        EXPECT_NE(url.find("state="), std::string::npos);
                        promise->set_value();
                    } catch(...) {
                        promise->set_exception(std::current_exception());
                    }
                });
            });
        }).get();
    }

    void testCallbackMissingState400() {
        runAsyncTest<void>([this](auto promise) {
            drogon::app().getLoop()->queueInLoop([this, promise]() {
                auto req = drogon::HttpRequest::newHttpRequest();
                controller.callback(req, [promise](const drogon::HttpResponsePtr &resp) {
                    try {
                        EXPECT_EQ(resp->getStatusCode(), drogon::k400BadRequest);
                        EXPECT_EQ(resp->contentType(), drogon::CT_APPLICATION_JSON);
                        const auto json = resp->getJsonObject();
                        ASSERT_NE(json, nullptr);
                        ASSERT_TRUE(json->isMember("error"));
                        EXPECT_EQ((*json)["error"].asString(), "Invalid or expired state parameter");
                        promise->set_value();
                    } catch(...) {
                        promise->set_exception(std::current_exception());
                    }
                });
            });
        }).get();
    }

    void testCallbackInvalidState400() {
        runAsyncTest<void>([this](auto promise) {
            drogon::app().getLoop()->queueInLoop([this, promise]() {
                auto req = drogon::HttpRequest::newHttpRequest();
                req->setParameter("state", "totally_wrong_state_value");
                controller.callback(req, [promise](const drogon::HttpResponsePtr &resp) {
                    try {
                        EXPECT_EQ(resp->getStatusCode(), drogon::k400BadRequest);
                        EXPECT_EQ(resp->contentType(), drogon::CT_APPLICATION_JSON);
                        const auto json = resp->getJsonObject();
                        ASSERT_NE(json, nullptr);
                        ASSERT_TRUE(json->isMember("error"));
                        EXPECT_EQ((*json)["error"].asString(), "Invalid or expired state parameter");
                        promise->set_value();
                    } catch(...) {
                        promise->set_exception(std::current_exception());
                    }
                });
            });
        }).get();
    }

    // Calls getOAuthUrl to seed pendingState, then calls callback with that state
    // but no code — verifies the "missing code" 400 path.
    void testCallbackValidStateMissingCode400() {
        std::string capturedState;

        runAsyncTest<void>([this, &capturedState](auto promise) {
            drogon::app().getLoop()->queueInLoop([this, promise, &capturedState]() {
                auto req = drogon::HttpRequest::newHttpRequest();
                controller.getOAuthUrl(req, [promise, &capturedState](const drogon::HttpResponsePtr &resp) {
                    try {
                        ASSERT_EQ(resp->getStatusCode(), drogon::k200OK);
                        const auto json = resp->getJsonObject();
                        ASSERT_NE(json, nullptr);
                        const std::string url = (*json)["url"].asString();
                        const auto statePos = url.find("state=");
                        ASSERT_NE(statePos, std::string::npos);
                        capturedState = url.substr(statePos + 6);
                        ASSERT_FALSE(capturedState.empty());
                        promise->set_value();
                    } catch(...) {
                        promise->set_exception(std::current_exception());
                    }
                });
            });
        }).get();

        runAsyncTest<void>([this, &capturedState](auto promise) {
            drogon::app().getLoop()->queueInLoop([this, promise, capturedState]() {
                auto req = drogon::HttpRequest::newHttpRequest();
                req->setParameter("state", capturedState);
                controller.callback(req, [promise](const drogon::HttpResponsePtr &resp) {
                    try {
                        EXPECT_EQ(resp->getStatusCode(), drogon::k400BadRequest);
                        EXPECT_EQ(resp->contentType(), drogon::CT_APPLICATION_JSON);
                        const auto json = resp->getJsonObject();
                        ASSERT_NE(json, nullptr);
                        ASSERT_TRUE(json->isMember("error"));
                        EXPECT_EQ((*json)["error"].asString(), "Missing code parameter");
                        promise->set_value();
                    } catch(...) {
                        promise->set_exception(std::current_exception());
                    }
                });
            });
        }).get();
    }
};

TEST_F(PinterestOAuthTest, GetOne200) {
    testGetOne200();
}

TEST_F(PinterestOAuthTest, GetList200) {
    testGetList200();
}

TEST_F(PinterestOAuthTest, GetOAuthUrl200) {
    testGetOAuthUrl200();
}

TEST_F(PinterestOAuthTest, CallbackMissingState400) {
    testCallbackMissingState400();
}

TEST_F(PinterestOAuthTest, CallbackInvalidState400) {
    testCallbackInvalidState400();
}

TEST_F(PinterestOAuthTest, CallbackValidStateMissingCode400) {
    testCallbackValidStateMissingCode400();
}
