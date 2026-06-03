#pragma once

#include "controllers/AiAnalyzeImage.h"
#include "utils/Base64.h"
#include "utils/config.h"

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
    std::filesystem::path promptFile;
    std::string originalPath;
    api::v1::AiAnalyzeImage controller;

protected:
    void SetUp() override {
        // mkdtemp creates the directory with 0700 perms — only this process's
        // user can list/read the stub script we drop inside it. The base path
        // is configurable via FOXY_AI_WORKDIR for environments where /tmp is
        // unsuitable.
        // S5443: /tmp default is fine in tests — mkdtemp creates a 0700 subdir
        // owned by the test process, deleted in TearDown.
        const std::string base = api::v1::getEnv("FOXY_AI_WORKDIR", "/tmp");  // NOSONAR(cpp:S5443)
        std::string templ = fmt::format("{}/foxy_ai_stub_XXXXXX", base);
        ASSERT_NE(::mkdtemp(templ.data()), nullptr);
        stubDir = templ;

        const auto scriptPath = stubDir / "claude";
        promptFile = stubDir / "prompt.txt";
        // The stub records every argument it receives (the prompt is the last
        // arg) so tests can assert what the controller asked claude to do.
        std::ofstream(scriptPath) << "#!/bin/sh\n"
                                     "printf '%s\\n' \"$@\" > \"$STUB_CLAUDE_PROMPT_FILE\"\n"
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
        setenv("STUB_CLAUDE_PROMPT_FILE", promptFile.c_str(), 1);

        unsetenv("STUB_CLAUDE_FAIL");
        unsetenv("STUB_CLAUDE_GARBAGE");
    }

    void TearDown() override {
        if(!originalPath.empty())
            setenv("PATH", originalPath.c_str(), 1);
        unsetenv("STUB_CLAUDE_FAIL");
        unsetenv("STUB_CLAUDE_GARBAGE");
        unsetenv("STUB_CLAUDE_PROMPT_FILE");
        std::error_code ec;
        std::filesystem::remove_all(stubDir, ec);
    }

    // Returns the full argument list the stub claude was invoked with, including
    // the prompt. Empty if claude was never called.
    [[nodiscard]] std::string capturedPrompt() const {
        std::ifstream in(promptFile);
        if(!in)
            return {};
        return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
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

    // Runs the request, asserts a 200 OK, and returns the full prompt the stub
    // claude was invoked with — the basis for all prompt-content assertions.
    std::string promptFor(const Json::Value &body) {
        invoke(buildRequest(body), [](const drogon::HttpResponsePtr &resp) {
            EXPECT_EQ(resp->getStatusCode(), drogon::k200OK);
        });
        return capturedPrompt();
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

TEST_F(AiAnalyzeImageTest, NoSizeOverridesKeepsDefaultWording) {
    const std::string prompt = promptFor(validBody());
    EXPECT_FALSE(prompt.empty());
    EXPECT_EQ(prompt.find("approximately"), std::string::npos);
    EXPECT_NE(prompt.find("short product title, 5-12 words"), std::string::npos);
    EXPECT_NE(prompt.find("2-4 sentence engaging product description"), std::string::npos);
    EXPECT_NE(prompt.find("SEO meta description, max 160 chars"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, DescriptionSizeOverrideInjected) {
    Json::Value body = validBody();
    body["description_size"] = 300;
    const std::string prompt = promptFor(body);
    EXPECT_NE(prompt.find("engaging product description, approximately 300 characters"), std::string::npos);
    // Other fields keep their default wording.
    EXPECT_NE(prompt.find("short product title, 5-12 words"), std::string::npos);
    EXPECT_NE(prompt.find("SEO meta description, max 160 chars"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, AllSizeOverridesInjected) {
    Json::Value body = validBody();
    body["title_size"] = 60;
    body["description_size"] = 300;
    body["meta_description_size"] = 150;
    const std::string prompt = promptFor(body);
    EXPECT_NE(prompt.find("product title, approximately 60 characters"), std::string::npos);
    EXPECT_NE(prompt.find("engaging product description, approximately 300 characters"), std::string::npos);
    EXPECT_NE(prompt.find("SEO meta description, approximately 150 characters"), std::string::npos);
    // No default wording remains for the overridden fields.
    EXPECT_EQ(prompt.find("5-12 words"), std::string::npos);
    EXPECT_EQ(prompt.find("2-4 sentence"), std::string::npos);
    EXPECT_EQ(prompt.find("max 160 chars"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, TitleSizeAboveMaxFallsBack) {
    Json::Value body = validBody();
    body["title_size"] = 200;  // > 60
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("title, approximately"), std::string::npos);
    EXPECT_NE(prompt.find("short product title, 5-12 words"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, DescriptionSizeAboveMaxFallsBack) {
    Json::Value body = validBody();
    body["description_size"] = 5000;  // > 1000
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("description, approximately"), std::string::npos);
    EXPECT_NE(prompt.find("2-4 sentence engaging product description"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, MetaDescriptionSizeAboveMaxFallsBack) {
    Json::Value body = validBody();
    body["meta_description_size"] = 500;  // > 160
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("meta description, approximately"), std::string::npos);
    EXPECT_NE(prompt.find("SEO meta description, max 160 chars"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, TitleSizeBelowMinFallsBack) {
    Json::Value body = validBody();
    body["title_size"] = 3;  // < 10
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("title, approximately"), std::string::npos);
    EXPECT_NE(prompt.find("short product title, 5-12 words"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, DescriptionSizeBelowMinFallsBack) {
    Json::Value body = validBody();
    body["description_size"] = 10;  // < 50
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("description, approximately"), std::string::npos);
    EXPECT_NE(prompt.find("2-4 sentence engaging product description"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, MetaDescriptionSizeBelowMinFallsBack) {
    Json::Value body = validBody();
    body["meta_description_size"] = 10;  // < 50
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("meta description, approximately"), std::string::npos);
    EXPECT_NE(prompt.find("SEO meta description, max 160 chars"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, TitleSizeBoundariesInject) {
    Json::Value max = validBody();
    max["title_size"] = 60;  // == max
    EXPECT_NE(promptFor(max).find("product title, approximately 60 characters"), std::string::npos);

    Json::Value min = validBody();
    min["title_size"] = 10;  // == min
    EXPECT_NE(promptFor(min).find("product title, approximately 10 characters"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, TitleSizeJustOutsideBoundariesFallBack) {
    Json::Value over = validBody();
    over["title_size"] = 61;  // max + 1
    EXPECT_EQ(promptFor(over).find("title, approximately"), std::string::npos);

    Json::Value under = validBody();
    under["title_size"] = 9;  // min - 1
    EXPECT_EQ(promptFor(under).find("title, approximately"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, MixedSizeOverridesInjectOnlyInRange) {
    Json::Value body = validBody();
    body["title_size"] = 50;  // in range → inject
    body["description_size"] = 5000;  // out of range → fall back
    // meta_description_size unset → fall back
    const std::string prompt = promptFor(body);
    EXPECT_NE(prompt.find("product title, approximately 50 characters"), std::string::npos);
    EXPECT_NE(prompt.find("2-4 sentence engaging product description"), std::string::npos);
    EXPECT_NE(prompt.find("SEO meta description, max 160 chars"), std::string::npos);
    EXPECT_EQ(prompt.find("description, approximately"), std::string::npos);
    EXPECT_EQ(prompt.find("meta description, approximately"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, NonPositiveSizeOverridesIgnored) {
    Json::Value body = validBody();
    body["title_size"] = 0;
    body["description_size"] = -10;
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("approximately"), std::string::npos);
    EXPECT_NE(prompt.find("short product title, 5-12 words"), std::string::npos);
    EXPECT_NE(prompt.find("2-4 sentence engaging product description"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, NonNumericSizeOverrideIgnored) {
    Json::Value body = validBody();
    body["description_size"] = "not a number";
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("approximately"), std::string::npos);
    EXPECT_NE(prompt.find("2-4 sentence engaging product description"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, NoExtraPromptOmitsBlock) {
    const std::string prompt = promptFor(validBody());
    EXPECT_FALSE(prompt.empty());
    EXPECT_EQ(prompt.find("Additional instructions from the request"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, ExtraPromptInjected) {
    Json::Value body = validBody();
    body["extra_prompt"] = "Focus on the brand name and keep the tone playful.";
    const std::string prompt = promptFor(body);
    EXPECT_NE(prompt.find("Additional instructions from the request"), std::string::npos);
    EXPECT_NE(prompt.find("Focus on the brand name and keep the tone playful."), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, EmptyExtraPromptIgnored) {
    Json::Value body = validBody();
    body["extra_prompt"] = "";
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("Additional instructions from the request"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, NonStringExtraPromptIgnored) {
    Json::Value body = validBody();
    body["extra_prompt"] = 123;
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("Additional instructions from the request"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, ExtraPromptAppearsAfterSizeOverrides) {
    Json::Value body = validBody();
    body["title_size"] = 60;
    body["extra_prompt"] = "Emphasize seasonal colors.";
    const std::string prompt = promptFor(body);
    const auto sizePos = prompt.find("product title, approximately 60 characters");
    const auto extraPos = prompt.find("Additional instructions from the request");
    ASSERT_NE(sizePos, std::string::npos);
    ASSERT_NE(extraPos, std::string::npos);
    EXPECT_LT(sizePos, extraPos);
}
