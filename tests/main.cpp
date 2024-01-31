// Tell DrogonTest to generate `test::run()`. Only defined this in the main file
#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include "drogon/drogon.h"
#include "json/json.h"
#include "../src/utils/time.h"

using namespace drogon;


DROGON_TEST(ListItems)
{
    auto client = HttpClient::newHttpClient("http://localhost:8848");
    auto req = HttpRequest::newHttpRequest();
    req->setPath("/api/v1/item/");
    client->sendRequest(req, [TEST_CTX](ReqResult res, const HttpResponsePtr& resp) {
        // There's nothing we can do if the request didn't reach the server
        // or the server generated garbage.
        REQUIRE(res == ReqResult::Ok);
        REQUIRE(resp != nullptr);

        CHECK(resp->getStatusCode() == k200OK);
        CHECK(resp->contentType() == CT_APPLICATION_JSON);
        auto respJson = *resp->jsonObject();
        CHECK(respJson["count"].asInt() == 12);
        CHECK(respJson["page"].asInt() == 1);

        // Check the first item as an example
        auto firstItem = respJson["items"][0];
        CHECK(firstItem["created_at"].asString() == "2024-01-21T22:02:21.197599");
        CHECK(firstItem["description"].asString() == "description description");
        CHECK(firstItem["item_id"].asInt() == 1);
        CHECK(firstItem["meta_description"].asString() == "meta_description");
        CHECK(firstItem["slug"].asString() == "breakfast");
        CHECK(firstItem["src"].asString() == "items/friend/tp-friend.jpg");
        CHECK(firstItem["title"].asString() == "Breakfast");
        CHECK(firstItem["updated_at"].asString() == "2024-01-21T22:02:21.197599");
    });
}

DROGON_TEST(CreateItem) {
    auto client = HttpClient::newHttpClient("http://localhost:8848");
    auto req = HttpRequest::newHttpRequest();
    req->setPath("/api/v1/item/");
    req->setMethod(drogon::Post);
    req->setContentTypeCode(CT_APPLICATION_JSON);
    std::string slug = "test" + std::to_string(getCurrentTimeSinceEpochInMilliseconds());
    req->setBody("{\"title\":\"Test\",\"description\":\"Test\",\"meta_description\":\"Test\",\"slug\":\"" + slug + "\"}");
    std::string token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhdWQiOiIiLCJlbWFpbCI6IjByYW5nZUZveEBkb21haW4ucHQiLCJleHAiOjE3MDczNTE0NDcsImlhdCI6MTcwNDc1OTQ0NywiaXNzIjoiIiwibmJmIjoxNzA0NzU5NDQ3fQ.FOwWkRQd4Q0-Kv7OOXTlJvmdIGDvvnf6854zrPEU7Z8";
    req->addHeader("Authorization", "Bearer " + token);
    client->sendRequest(req, [TEST_CTX, slug](ReqResult res, const HttpResponsePtr& resp) {
        // There's nothing we can do if the request didn't reach the server
        // or the server generated garbage.
        REQUIRE(res == ReqResult::Ok);
        REQUIRE(resp != nullptr);
        std::cout << slug<< std::endl;

        CHECK(resp->getStatusCode() == drogon::k201Created);
        CHECK(resp->contentType() == CT_APPLICATION_JSON);
        auto respJson = *resp->jsonObject();
        CHECK(respJson["title"].asString() == "Test");
        CHECK(respJson["description"].asString() == "Test");
        CHECK(respJson["meta_description"].asString() == "Test");
        CHECK(respJson["slug"].asString() == slug);
    });
}

int main(int argc, char** argv)
{
    std::promise<void> p1;
    std::future<void> f1 = p1.get_future();

    // Start the main loop on another thread
    std::thread thr([&]() {
        // Queues the promise to be fulfilled after starting the loop
        app().getLoop()->queueInLoop([&p1]() { p1.set_value(); });
        app().run();
    });

    // The future is only satisfied after the event loop started
    f1.get();
    int status = test::run(argc, argv);

    // Ask the event loop to shutdown and wait
    app().getLoop()->queueInLoop([]() { app().quit(); });
    thr.join();
    return status;
}
