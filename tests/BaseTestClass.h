#pragma once

#include "BaseClass.h"
#include "drogon/HttpRequest.h"
#include "drogon/drogon.h"
#include <ranges>
#include "env.h"

#include <future>
#include <gtest/gtest.h>
#include "fmt/core.h"

template<class ControllerTest, class Controller>
class BaseTestClass : public api::v1::BaseClass, public testing::Test {
public:
    void SetUp() override {
        req = drogon::HttpRequest::newHttpRequest();
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
    Json::Value expectedValues = {};
    Json::Value updatedValues = {};
    Json::Value getOneValues = {};
    Json::Value getListValues = {};

    virtual void setupExpectedValues() = 0;
    virtual void setupUpdatedValues() = 0;
    virtual void setupGetOneValues() = 0;
    virtual void setupGetListValues() = 0;

    static void
    checkJsonValue(const Json::Value& respJson, const Json::Value& expectedValue, const std::string& keyPath = "") {
        // Helper function to build key paths for error messages
        auto buildKeyPath = [&](const std::string& subKey) {
            return keyPath.empty() ? subKey : keyPath + "." + subKey;
        };

        // Check for type mismatch
        ASSERT_EQ(respJson.type(), expectedValue.type()) << "Type mismatch at key path: " << keyPath;

        if(respJson.isObject()) {
            // Check all keys in the expected object
            for(const auto& subKey: expectedValue.getMemberNames()) {
                ASSERT_TRUE(respJson.isMember(subKey))
                    << "Missing key '" << buildKeyPath(subKey) << "' in the actual JSON.";
                checkJsonValue(respJson[subKey], expectedValue[subKey], buildKeyPath(subKey));
            }
        } else if(respJson.isArray()) {
            // Check array size
            ASSERT_EQ(respJson.size(), expectedValue.size()) << "Array size mismatch at key path: " << keyPath;

            // Check each element in the array
            for(Json::ArrayIndex i = 0; i < respJson.size(); ++i) {
                checkJsonValue(respJson[i], expectedValue[i], buildKeyPath("[" + std::to_string(i) + "]"));
            }
        } else if(keyPath.ends_with("src")) {
            // Special handling for "src"
            EXPECT_EQ(respJson.asString(), fmt::format("https://{}/{}", APP_CLOUD_NAME, expectedValue.asString()))
                << "Mismatch at key path: " << keyPath;
        } else {
            // Direct value comparison
            EXPECT_EQ(respJson, expectedValue) << "Value mismatch at key path: " << keyPath;
        }
    }

    std::function<void(const drogon::HttpResponsePtr&)>
    updateCallback(const std::shared_ptr<std::promise<void>>& testPromise, const drogon::orm::DbClientPtr& dbClient) {
        return [this, dbClient, testPromise](const drogon::HttpResponsePtr& resp) {
            try {
                EXPECT_EQ(resp->contentType(), drogon::CT_APPLICATION_JSON);
                EXPECT_EQ(resp->getStatusCode(), drogon::k200OK);

                const auto responseJson = resp->getJsonObject();
                const Json::StreamWriterBuilder builder;
                const std::string jsonString = writeString(builder, *responseJson);
                std::cout << jsonString << std::endl;
                if(resp->getStatusCode() == drogon::k200OK) {
                    this->checkJsonValue(*responseJson, updatedValues);
                }
                *dbClient << "ROLLBACK;";
                testPromise->set_value();
            } catch(const std::exception& e) {
                testPromise->set_exception(std::current_exception());
                LOG_ERROR << e.what();
            }
        };
    }

    std::function<void(const drogon::HttpResponsePtr&)>
    getOneCallback(const std::shared_ptr<std::promise<void>>& testPromise) {
        return [this, testPromise](const drogon::HttpResponsePtr& resp) {
            try {
                EXPECT_EQ(resp->contentType(), drogon::CT_APPLICATION_JSON);
                EXPECT_EQ(resp->getStatusCode(), drogon::k200OK);

                const auto responseJson = resp->getJsonObject();
                const Json::StreamWriterBuilder builder;
                const std::string jsonString = writeString(builder, *responseJson);
                std::cout << jsonString << std::endl;
                if(resp->getStatusCode() == drogon::k200OK) {
                    this->checkJsonValue(*responseJson, getOneValues);
                }
                testPromise->set_value();
            } catch(const std::exception& e) {
                testPromise->set_exception(std::current_exception());
                LOG_ERROR << e.what();
            }
        };
    }

    std::function<void(const drogon::HttpResponsePtr&)>
    getListCallback(const std::shared_ptr<std::promise<void>>& testPromise) {
        return [this, testPromise](const drogon::HttpResponsePtr& resp) {
            try {
                EXPECT_EQ(resp->contentType(), drogon::CT_APPLICATION_JSON);
                EXPECT_EQ(resp->getStatusCode(), drogon::k200OK);

                const auto responseJson = resp->getJsonObject();
                const Json::StreamWriterBuilder builder;
                const std::string jsonString = writeString(builder, *responseJson);
                std::cout << jsonString << std::endl;
                if(resp->getStatusCode() == drogon::k200OK) {
                    this->checkJsonValue(*responseJson, getListValues);
                }
                testPromise->set_value();
            } catch(const std::exception& e) {
                testPromise->set_exception(std::current_exception());
                LOG_ERROR << e.what();
            }
        };
    }

    std::function<void(const drogon::HttpResponsePtr&)>
    createCallback(const std::shared_ptr<std::promise<void>>& testPromise, const drogon::orm::DbClientPtr& dbClient) {
        return [this, dbClient, testPromise](const drogon::HttpResponsePtr& resp) {
            try {
                EXPECT_EQ(resp->contentType(), drogon::CT_APPLICATION_JSON);
                EXPECT_EQ(resp->getStatusCode(), drogon::k201Created);

                const auto responseJson = resp->getJsonObject();
                const Json::StreamWriterBuilder builder;
                const std::string jsonString = writeString(builder, *responseJson);
                std::cout << jsonString << std::endl;
                if(resp->getStatusCode() == drogon::k201Created) {
                    this->checkJsonValue(*responseJson, expectedValues);
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
    emptyBodyCallback(const std::shared_ptr<std::promise<void>>& testPromise) {
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

    [[nodiscard]] std::function<void(const drogon::HttpResponsePtr&)>
    requiredFieldsCallback(const std::shared_ptr<std::promise<void>>& testPromise) const {
        return [this, testPromise](const drogon::HttpResponsePtr& resp) {
            try {
                EXPECT_EQ(resp->contentType(), drogon::CT_APPLICATION_JSON);
                EXPECT_EQ(resp->getStatusCode(), drogon::k400BadRequest);

                const auto responseJson = resp->getJsonObject();
                const Json::StreamWriterBuilder builder;
                const std::string jsonString = writeString(builder, *responseJson);
                std::cout << jsonString << std::endl;
                auto filteredKeys = expectedValues.getMemberNames() | std::views::filter([](const std::string& key) {
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

    [[nodiscard]] std::function<void(const drogon::HttpResponsePtr&)>
    deleteItemCallback(const std::shared_ptr<std::promise<void>>& testPromise,
                       const drogon::orm::DbClientPtr& dbClient) const {
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
        setupExpectedValues();
        req->setBody(expectedValues.toStyledString());

        runAsyncTest<void>([this](auto promise) {
            drogon::app().getLoop()->queueInLoop([this, promise]() {
                const auto dbClient = drogon::app().getFastDbClient("default");
                *dbClient << "BEGIN";
                controller.createItem(*reqPtr, createCallback(promise, dbClient));
            });
        }).get();
    }

    void testEmptyBody400() {
        runAsyncTest<void>([this](auto promise) {
            drogon::app().getLoop()->queueInLoop([this, promise]() {
                controller.createItem(*reqPtr, emptyBodyCallback(promise));
            });
        }).get();
    }

    void testRequiredFields400() {
        setupExpectedValues();
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
        setupUpdatedValues();
        req->setBody(updatedValues.toStyledString());

        runAsyncTest<void>([this](auto promise) {
            drogon::app().getLoop()->queueInLoop([this, promise]() {
                const auto dbClient = drogon::app().getFastDbClient("default");
                *dbClient << "BEGIN";
                controller.updateItem(*reqPtr, updateCallback(promise, dbClient), "1");
            });
        }).get();
    }

    void getOne200() {
        setupGetOneValues();

        runAsyncTest<void>([this](auto promise) {
            drogon::app().getLoop()->queueInLoop([this, promise]() {
                controller.getOne(*reqPtr, getOneCallback(promise), "1");
            });
        }).get();
    }

    void getList200() {
        setupGetListValues();

        runAsyncTest<void>([this](auto promise) {
            drogon::app().getLoop()->queueInLoop([this, promise]() {
                controller.getList(*reqPtr, getListCallback(promise));
            });
        }).get();
    }
};
