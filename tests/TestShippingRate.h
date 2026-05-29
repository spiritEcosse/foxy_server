#pragma once

#include "BaseTestClass.h"
#include "controllers/ShippingRate.h"

#include <gtest/gtest.h>

class ShippingRateControllerTest : public BaseTestClass<ShippingRateControllerTest, api::v1::ShippingRate> {
protected:
    void setupExpectedValues() override {
        expectedValues["shipping_profile_id"] = 1;
        expectedValues["delivery_days_min"] = 1;
        expectedValues["delivery_days_max"] = 1;
    }

    void setupUpdatedValues() override {
        updatedValues["shipping_profile_id"] = 2;
        updatedValues["delivery_days_min"] = 2;
        updatedValues["delivery_days_max"] = 2;
    }

    void setupGetOneValues() override {
        getOneValues["country_id"] = 1;
        getOneValues["delivery_days_max"] = 5;
        getOneValues["delivery_days_min"] = 1;
        getOneValues["id"] = 1;
        getOneValues["shipping_profile_id"] = 1;
    }

    void setupGetListValues() override {
        getListValues["_page"] = 1;
        getListValues["total"] = 3;

        Json::Value data = Json::arrayValue;

        Json::Value entry1;
        entry1["country_id"] = Json::nullValue;
        entry1["id"] = 3;
        entry1["delivery_days_max"] = 5;
        entry1["delivery_days_min"] = 1;
        entry1["shipping_profile_id"] = 1;

        Json::Value entry2;
        entry2["country_id"] = 2;
        entry2["id"] = 2;
        entry2["delivery_days_max"] = 6;
        entry2["delivery_days_min"] = 2;
        entry2["shipping_profile_id"] = 2;

        Json::Value entry3;
        entry3["country_id"] = 1;
        entry3["id"] = 1;
        entry3["delivery_days_max"] = 5;
        entry3["delivery_days_min"] = 1;
        entry3["shipping_profile_id"] = 1;

        data.append(entry1);
        data.append(entry2);
        data.append(entry3);

        getListValues["data"] = data;
    }

    void checkShippingRateByItemResponse(const drogon::HttpResponsePtr& resp) {
        EXPECT_EQ(resp->contentType(), drogon::CT_APPLICATION_JSON);
        const auto responseJson = resp->getJsonObject();
        const Json::StreamWriterBuilder builder;
        std::cout << writeString(builder, *responseJson) << std::endl;
        Json::Value obj;
        Json::Value shipping;
        shipping["delivery_days_max"] = 7;
        shipping["delivery_days_min"] = 3;
        obj["shipping"] = shipping;
        this->checkJsonValue(*responseJson, obj);
    }

    std::function<void(const drogon::HttpResponsePtr&)>
    getShippingRateByItemCallback(const std::shared_ptr<std::promise<void>>& testPromise,
                                  const drogon::HttpStatusCode statusCode = drogon::k200OK) {
        return [this, testPromise, statusCode](const drogon::HttpResponsePtr& resp) {
            EXPECT_EQ(resp->getStatusCode(), statusCode);
            if(resp->getStatusCode() == drogon::k200OK)
                checkShippingRateByItemResponse(resp);
            testPromise->set_value();
        };
    }
};

TEST_F(ShippingRateControllerTest, Create200) {
    testCreate200();
}

TEST_F(ShippingRateControllerTest, EmptyBody400) {
    testEmptyBody400();
}

TEST_F(ShippingRateControllerTest, RequiredFields400) {
    testRequiredFields400();
}

TEST_F(ShippingRateControllerTest, Delete204) {
    testDelete204();
}

TEST_F(ShippingRateControllerTest, Update200) {
    testUpdate200();
}

TEST_F(ShippingRateControllerTest, GetOne200) {
    testGetOne200();
}

TEST_F(ShippingRateControllerTest, GetShippingRateByItem) {
    setupGetOneValues();

    runAsyncTest<void>([this](auto promise) {
        drogon::app().getLoop()->queueInLoop([this, promise]() {
            controller.getShippingRateByItem(*reqPtr, getShippingRateByItemCallback(promise), "item1");
        });
    }).get();
}

TEST_F(ShippingRateControllerTest, GetList200) {
    testGetList200();
}

TEST_F(ShippingRateControllerTest, GetOne404) {
    testGetOne404();
}

TEST_F(ShippingRateControllerTest, testDeleteItems) {
    testDeleteItems();
}

TEST_F(ShippingRateControllerTest, testCreateItems) {
    testCreateItems();
}

TEST_F(ShippingRateControllerTest, testUpdateItems) {
    testUpdateItems();
}
