#pragma once

#include "controllers/AiAnalyzeImage.h"
#include "utils/Base64.h"

#include "drogon/HttpRequest.h"
#include "drogon/drogon.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <future>
#include <gtest/gtest.h>
#include <string>
#include <sys/stat.h>

class AiAnalyzeImageTest : public ::testing::Test {
private:
    std::filesystem::path stubDir;
    std::string originalPath;
    api::v1::AiAnalyzeImage controller;

protected:
    void SetUp() override {
        // mkdtemp creates the directory with 0700 perms — only this process's
        // user can list/read the stub script we drop inside it.
        std::string templ = (std::filesystem::temp_directory_path() / "foxy_ai_stub_XXXXXX").string();
        ASSERT_NE(::mkdtemp(templ.data()), nullptr);
        stubDir = templ;

        const auto scriptPath = stubDir / "claude";
        std::ofstream(scriptPath) << "#!/bin/sh\n"
                                     "if [ -n \"$STUB_CLAUDE_FAIL\" ]; then echo \"boom\" 1>&2; exit 1; fi\n"
                                     "if [ -n \"$STUB_CLAUDE_GARBAGE\" ]; then echo \"not json\"; exit 0; fi\n"
                                     "cat <<'EOF'\n"
                                     "{\"title\":\"t\",\"description\":\"d\",\"meta_description\":\"m\","
                                     "\"tags\":[{\"title\":\"X\",\"social_media\":[\"Instagram\"]}]}\n"
                                     "EOF\n";
        // Owner rwx; no permissions for group/others (S2612).
        ::chmod(scriptPath.c_str(), S_IRWXU);

        if(const char *p = std::getenv("PATH"))
            originalPath = p;
        setenv("PATH", fmt::format("{}:{}", stubDir.string(), originalPath).c_str(), 1);

        unsetenv("STUB_CLAUDE_FAIL");
        unsetenv("STUB_CLAUDE_GARBAGE");
    }

    void TearDown() override {
        if(!originalPath.empty())
            setenv("PATH", originalPath.c_str(), 1);
        unsetenv("STUB_CLAUDE_FAIL");
        unsetenv("STUB_CLAUDE_GARBAGE");
        std::error_code ec;
        std::filesystem::remove_all(stubDir, ec);
    }

    static std::string sampleImageBase64() {
        // Content is irrelevant — the stub script ignores it.
        const std::string bytes(8, '\x{00}');
        return Base64::Encode(bytes);
    }

    static drogon::HttpRequestPtr buildRequest(const Json::Value &body) {
        auto req = drogon::HttpRequest::newHttpRequest();
        req->setMethod(drogon::Post);
        req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        req->setBody(body.toStyledString());
        return req;
    }

    static Json::Value validBody() {
        Json::Value body;
        body["image"] = sampleImageBase64();
        body["mimeType"] = "image/png";
        return body;
    }

    using Check = std::function<void(const drogon::HttpResponsePtr &)>;

    void invoke(const drogon::HttpRequestPtr &req, const Check &check) {
        auto promise = std::make_shared<std::promise<void>>();
        auto future = promise->get_future();
        drogon::app().getLoop()->queueInLoop([this, req, check, promise]() {
            controller.analyze(req, [check, promise](const drogon::HttpResponsePtr &resp) {
                try {
                    check(resp);
                    promise->set_value();
                } catch(...) {
                    promise->set_exception(std::current_exception());
                }
            });
        });
        future.get();
    }

    static Check expectStatusAndError(drogon::HttpStatusCode code, const std::string &expectedError) {
        return [code, expectedError](const drogon::HttpResponsePtr &resp) {
            EXPECT_EQ(resp->getStatusCode(), code);
            const auto json = resp->getJsonObject();
            ASSERT_NE(json, nullptr);
            EXPECT_EQ((*json)["error"].asString(), expectedError);
        };
    }
};

TEST_F(AiAnalyzeImageTest, Analyze200) {
    invoke(buildRequest(validBody()), [](const drogon::HttpResponsePtr &resp) {
        EXPECT_EQ(resp->getStatusCode(), drogon::k200OK);
        EXPECT_EQ(resp->contentType(), drogon::CT_APPLICATION_JSON);
        const auto json = resp->getJsonObject();
        ASSERT_NE(json, nullptr);
        EXPECT_EQ((*json)["title"].asString(), "t");
        EXPECT_EQ((*json)["description"].asString(), "d");
        EXPECT_EQ((*json)["meta_description"].asString(), "m");
        ASSERT_TRUE((*json)["tags"].isArray());
        ASSERT_EQ((*json)["tags"].size(), 1u);
        EXPECT_EQ((*json)["tags"][0]["title"].asString(), "X");
    });
}

TEST_F(AiAnalyzeImageTest, MissingImage400) {
    Json::Value body;
    body["mimeType"] = "image/png";
    invoke(buildRequest(body), expectStatusAndError(drogon::k400BadRequest, "Missing image or mimeType"));
}

TEST_F(AiAnalyzeImageTest, BadMimeType400) {
    Json::Value body = validBody();
    body["mimeType"] = "text/plain";
    invoke(buildRequest(body), expectStatusAndError(drogon::k400BadRequest, "Unsupported mimeType"));
}

TEST_F(AiAnalyzeImageTest, ClaudeFails502) {
    setenv("STUB_CLAUDE_FAIL", "1", 1);
    invoke(buildRequest(validBody()), expectStatusAndError(drogon::k502BadGateway, "claude CLI failed"));
}

TEST_F(AiAnalyzeImageTest, ClaudeBadJson502) {
    setenv("STUB_CLAUDE_GARBAGE", "1", 1);
    invoke(buildRequest(validBody()), expectStatusAndError(drogon::k502BadGateway, "Invalid JSON from claude"));
}
