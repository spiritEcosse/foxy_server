#pragma once

#include "BaseClass.h"
#include "drogon/HttpRequest.h"
#include "TransparentHasher.h"
#include "drogon/drogon.h"
#include <ranges>
#include "env.h"

#include <future>
#include <gtest/gtest.h>
#include "fmt/core.h"

using VariantFields = std::variant<int, bool, std::string, double>;
using FieldsMap =
    decltype(std::unordered_map<std::string, VariantFields, api::v1::TransparentHasher, std::equal_to<>>());

template<class ControllerTest, class Controller>
class BaseTestClass : public api::v1::BaseClass, public testing::Test {
public:
    void SetUp() override {
        req = drogon::HttpRequest::newHttpRequest();
        req->setBody(body());
        req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        reqPtr = std::make_shared<drogon::HttpRequestPtr>(req);
    }

    void TearDown() override {
        req.reset();
        reqPtr.reset();
    }

    std::shared_ptr<drogon::HttpRequest> req;
    std::shared_ptr<drogon::HttpRequestPtr> reqPtr;
    Controller controller;

    static std::string body() {
        Json::Value jsonValue;

        for(const auto& [key, value]: ControllerTest::expectedValues) {
            std::visit(
                [&](const auto& arg) {
                    jsonValue[key] = arg;
                },
                value);
        }
        const Json::StreamWriterBuilder writer;
        return writeString(writer, jsonValue);
    }

    static std::string updatedBody() {
        // Create a mutable copy of the expected values
        ControllerTest::updatedValues = ControllerTest::expectedValues;
        Json::Value jsonValue;

        for(auto& [key, value]: ControllerTest::updatedValues) {
            std::visit(
                [&]<typename T0>(const T0& arg) {
                    using T = std::decay_t<T0>;
                    if constexpr(std::is_same_v<T, int>) {
                        value = std::variant<int, bool, std::string, double>(2);
                    } else if constexpr(std::is_same_v<T, bool>) {
                        value = std::variant<int, bool, std::string, double>(false);
                    } else if constexpr(std::is_same_v<T, std::string>) {
                        value = std::variant<int, bool, std::string, double>(std::string("new string"));
                    } else if constexpr(std::is_same_v<T, double>) {
                        value = std::variant<int, bool, std::string, double>(123.45);
                    }
                    // Update the JSON value after updating the variant
                    std::visit(
                        [&](const auto& updated_arg) {
                            jsonValue[key] = updated_arg;
                        },
                        value);
                },
                value);
        }
        const Json::StreamWriterBuilder writer;
        return Json::writeString(writer, jsonValue);
    }

    template<typename T>
    void checkJsonValue(const Json::Value& respJson, const std::string& key, const T& expectedValue) const {
        if constexpr(std::is_same_v<T, int>) {
            EXPECT_EQ(respJson[key].asInt(), expectedValue);
        } else if constexpr(std::is_same_v<T, bool>) {
            EXPECT_EQ(respJson[key].asBool(), expectedValue);
        } else if constexpr(std::is_same_v<T, std::string>) {
            if(key == "src") {
                EXPECT_EQ(respJson[key].asString(), fmt::format("https://{}/{}", APP_CLOUD_NAME, expectedValue));
            } else {
                EXPECT_EQ(respJson[key].asString(), expectedValue);
            }
        } else if constexpr(std::is_same_v<T, double>) {
            EXPECT_EQ(respJson[key].asDouble(), expectedValue);
        } else {
            // Handle unexpected types or throw an exception
            FAIL();
        }
    }

    void
    checkJsonValueVariant(const Json::Value& respJson, const std::string& key, const VariantFields& expectedValue) {
        std::visit(
            [&](const auto& arg) {
                checkJsonValue(respJson, key, arg);
            },
            expectedValue);
    }

    std::function<void(const drogon::HttpResponsePtr&)> updateCallback(std::shared_ptr<std::promise<void>> testPromise,
                                                                       drogon::orm::DbClientPtr dbClient) {
        return [this, dbClient, testPromise](const drogon::HttpResponsePtr& resp) {
            try {
                EXPECT_EQ(resp->contentType(), drogon::CT_APPLICATION_JSON);
                EXPECT_EQ(resp->getStatusCode(), drogon::k200OK);

                const auto responseJson = resp->getJsonObject();
                const Json::StreamWriterBuilder builder;
                const std::string jsonString = writeString(builder, *responseJson);
                std::cout << jsonString << std::endl;
                if(resp->getStatusCode() == drogon::k200OK) {
                    for(const auto& [key, value]: ControllerTest::updatedValues) {
                        this->checkJsonValueVariant(*responseJson, key, value);
                    }
                }
                *dbClient << "ROLLBACK;";
                testPromise->set_value();
            } catch(const std::exception& e) {
                testPromise->set_exception(std::current_exception());
                LOG_ERROR << e.what();
            }
        };
    }

    std::function<void(const drogon::HttpResponsePtr&)> createCallback(std::shared_ptr<std::promise<void>> testPromise,
                                                                       drogon::orm::DbClientPtr dbClient) {
        return [this, dbClient, testPromise](const drogon::HttpResponsePtr& resp) {
            try {
                EXPECT_EQ(resp->contentType(), drogon::CT_APPLICATION_JSON);
                EXPECT_EQ(resp->getStatusCode(), drogon::k201Created);

                const auto responseJson = resp->getJsonObject();
                const Json::StreamWriterBuilder builder;
                const std::string jsonString = writeString(builder, *responseJson);
                std::cout << jsonString << std::endl;
                if(resp->getStatusCode() == drogon::k201Created) {
                    for(const auto& [key, value]: ControllerTest::expectedValues) {
                        this->checkJsonValueVariant(*responseJson, key, value);
                    }
                }
                *dbClient << "ROLLBACK;";
                testPromise->set_value();
            } catch(const std::exception& e) {
                testPromise->set_exception(std::current_exception());
                LOG_ERROR << e.what();
            }
        };
    }

    static std::function<void(const drogon::HttpResponsePtr&)>
    emptyBodyCallback(std::shared_ptr<std::promise<void>> testPromise) {
        return [testPromise](const drogon::HttpResponsePtr& resp) {
            try {
                EXPECT_EQ(resp->contentType(), drogon::CT_APPLICATION_JSON);
                EXPECT_EQ(resp->getStatusCode(), drogon::k400BadRequest);

                const auto responseJson = resp->getJsonObject();
                const Json::StreamWriterBuilder builder;
                const std::string jsonString = writeString(builder, *responseJson);
                std::cout << jsonString << std::endl;
                Json::Value expected;
                expected["error"] = "Empty body";
                EXPECT_EQ(*responseJson, expected);
                testPromise->set_value();
            } catch(const std::exception& e) {
                testPromise->set_exception(std::current_exception());
            }
        };
    }

    std::function<void(const drogon::HttpResponsePtr&)>
    requiredFieldsCallback(std::shared_ptr<std::promise<void>> testPromise) const {
        return [this, testPromise](const drogon::HttpResponsePtr& resp) {
            try {
                EXPECT_EQ(resp->contentType(), drogon::CT_APPLICATION_JSON);
                EXPECT_EQ(resp->getStatusCode(), drogon::k400BadRequest);

                const auto responseJson = resp->getJsonObject();
                const Json::StreamWriterBuilder builder;
                const std::string jsonString = writeString(builder, *responseJson);
                std::cout << jsonString << std::endl;
                auto filteredKeys = std::views::keys(ControllerTest::expectedValues) |
                                    std::views::filter([](const std::string_view& key) {
                                        return key != "id" && key != "enabled" && key != "status";
                                    });

                for(const auto& key: filteredKeys) {
                    EXPECT_EQ(responseJson->operator[](key).asString(), fmt::format("{} is required", key));
                }
                testPromise->set_value();
            } catch(const std::exception& e) {
                testPromise->set_exception(std::current_exception());
            }
        };
    }

    std::function<void(const drogon::HttpResponsePtr&)>
    deleteItemCallback(std::shared_ptr<std::promise<void>> testPromise, drogon::orm::DbClientPtr dbClient) const {
        return [this, testPromise, dbClient](const drogon::HttpResponsePtr& resp) {
            try {
                EXPECT_EQ(resp->contentType(), drogon::CT_APPLICATION_JSON);
                EXPECT_EQ(resp->getStatusCode(), drogon::k204NoContent);

                *dbClient << "ROLLBACK;";
                testPromise->set_value();
            } catch(const std::exception& e) {
                testPromise->set_exception(std::current_exception());
                LOG_ERROR << e.what();
            }
        };
    }

    template<typename T>
    std::future<T> runAsyncTest(std::function<void(std::shared_ptr<std::promise<T>>)> asyncOperation) {
        auto promise = std::make_shared<std::promise<T>>();
        auto future = promise->get_future();
        asyncOperation(promise);
        return future;
    }

    void testCreate200() {
        runAsyncTest<void>([this](auto promise) {
            drogon::app().getLoop()->queueInLoop([this, promise]() {
                const auto dbClient = drogon::app().getFastDbClient("default");
                *dbClient << "BEGIN";
                controller.createItem(*reqPtr, createCallback(promise, dbClient));
            });
        }).get();
    }

    void testEmptyBody400() {
        req->setBody(std::string());
        runAsyncTest<void>([this](auto promise) {
            drogon::app().getLoop()->queueInLoop([this, promise]() {
                controller.createItem(*reqPtr, emptyBodyCallback(promise));
            });
        }).get();
    }

    void testRequiredFields400() {
        req->setBody(std::string("{}"));
        runAsyncTest<void>([this](auto promise) {
            drogon::app().getLoop()->queueInLoop([this, promise]() {
                controller.createItem(*reqPtr, requiredFieldsCallback(promise));
            });
        }).get();
    }

    void testDelete204() {
        runAsyncTest<void>([this](auto promise) {
            drogon::app().getLoop()->queueInLoop([this, promise]() {
                const auto dbClient = drogon::app().getFastDbClient("default");
                *dbClient << "BEGIN";
                controller.deleteItem(*reqPtr, deleteItemCallback(promise, dbClient), "1");
            });
        }).get();
    }

    void testUpdate200() {
        runAsyncTest<void>([this](auto promise) {
            drogon::app().getLoop()->queueInLoop([this, promise]() {
                req->setBody(updatedBody());
                const auto dbClient = drogon::app().getFastDbClient("default");
                *dbClient << "BEGIN";
                controller.updateItem(*reqPtr, updateCallback(promise, dbClient), "1");
            });
        }).get();
    }
};
