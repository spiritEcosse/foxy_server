#pragma once

#include <gtest/gtest.h>
#include "clients/models/Pin.h"

class PinSerializationTest : public testing::Test {
protected:
    static api::v1::SharedFileTransferInfo makeImageMedia() {
        return std::make_shared<api::v1::FileTransferInfo>(
            "http://example.com/img.png",
            "/tmp/test_pin_image.png",
            "image",
            "image/png");
    }
};

TEST_F(PinSerializationTest, ToJsonSingleImage) {
    std::vector<api::v1::SharedFileTransferInfo> media;
    media.push_back(makeImageMedia());

    Json::Value tags = Json::arrayValue;
    api::v1::Pin pin(1, "Test Title", "test-slug", "Test description", media, tags);

    const std::string json = pin.toJson();
    ASSERT_FALSE(json.empty());

    Json::Value parsed;
    Json::Reader reader;
    ASSERT_TRUE(reader.parse(json, parsed));

    EXPECT_TRUE(parsed.isMember("title"));
    EXPECT_TRUE(parsed.isMember("description"));
    EXPECT_TRUE(parsed.isMember("board_id"));
    ASSERT_TRUE(parsed.isMember("media_source"));
    EXPECT_EQ(parsed["media_source"]["source_type"].asString(), "image_base64");
    EXPECT_TRUE(parsed["media_source"].isMember("data"));
    EXPECT_EQ(parsed["media_source"]["content_type"].asString(), "image/png");
}

TEST_F(PinSerializationTest, ToJsonMultipleImages) {
    std::vector<api::v1::SharedFileTransferInfo> media;
    media.push_back(makeImageMedia());
    media.push_back(makeImageMedia());

    Json::Value tags = Json::arrayValue;
    api::v1::Pin pin(1, "Test Title", "test-slug", "Test description", media, tags);

    const std::string json = pin.toJson();
    ASSERT_FALSE(json.empty());

    Json::Value parsed;
    Json::Reader reader;
    ASSERT_TRUE(reader.parse(json, parsed));

    ASSERT_TRUE(parsed.isMember("media_source"));
    EXPECT_EQ(parsed["media_source"]["source_type"].asString(), "multiple_image_base64");
    ASSERT_TRUE(parsed["media_source"]["items"].isArray());
    EXPECT_EQ(parsed["media_source"]["items"].size(), 2u);
    EXPECT_TRUE(parsed["media_source"]["items"][0].isMember("data"));
    EXPECT_TRUE(parsed["media_source"]["items"][0].isMember("content_type"));
    EXPECT_TRUE(parsed["media_source"]["items"][0].isMember("link"));
    EXPECT_EQ(parsed["media_source"]["items"][0]["content_type"].asString(), "image/png");
}
