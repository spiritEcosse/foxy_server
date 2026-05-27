#pragma once

#include "drogon/drogon.h"
#include "drogon/HttpClient.h"
#include "utils/config.h"
#include <gtest/gtest.h>
#include <future>

class CorsTest : public ::testing::Test {
protected:
    void SetUp() override {
        foxyClient = api::v1::getEnv("FOXY_CLIENT", "http://localhost:5173");
        client = drogon::HttpClient::newHttpClient("http://127.0.0.1:18080");
    }
    std::string foxyClient;
    drogon::HttpClientPtr client;

    void sendAndCheck(drogon::HttpMethod method,
                      const std::string &origin,
                      drogon::HttpStatusCode expectedStatus,
                      bool expectCorsHeader,
                      bool expectPreflightHeaders = false) {
        auto promise = std::make_shared<std::promise<void>>();
        auto future = promise->get_future();

        auto req = drogon::HttpRequest::newHttpRequest();
        req->setMethod(method);
        req->setPath("/api/v1/does-not-exist");
        req->addHeader("Origin", origin);

        client->sendRequest(req, [this, promise, expectedStatus, expectCorsHeader, expectPreflightHeaders]
                                 (drogon::ReqResult result, const drogon::HttpResponsePtr &resp) {
            try {
                ASSERT_EQ(result, drogon::ReqResult::Ok);
                EXPECT_EQ(resp->getStatusCode(), expectedStatus);
                if(expectCorsHeader) {
                    EXPECT_EQ(resp->getHeader("Access-Control-Allow-Origin"), foxyClient);
                } else {
                    EXPECT_TRUE(resp->getHeader("Access-Control-Allow-Origin").empty());
                }
                if(expectPreflightHeaders) {
                    EXPECT_FALSE(resp->getHeader("Access-Control-Allow-Methods").empty());
                    EXPECT_FALSE(resp->getHeader("Access-Control-Allow-Headers").empty());
                }
                promise->set_value();
            } catch(...) {
                promise->set_exception(std::current_exception());
            }
        });

        future.get();
    }
};

TEST_F(CorsTest, NotFoundPostHasCorsHeader) {
    sendAndCheck(drogon::Post, foxyClient, drogon::k404NotFound, true);
}

TEST_F(CorsTest, OptionsPreflightNotFoundReturns204WithCorsHeaders) {
    sendAndCheck(drogon::Options, foxyClient, drogon::k204NoContent, true, true);
}

TEST_F(CorsTest, UnknownOriginGetsNoCorsHeader) {
    sendAndCheck(drogon::Post, "http://evil.example.com", drogon::k404NotFound, false);
}
