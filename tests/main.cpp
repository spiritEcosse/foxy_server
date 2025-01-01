#include <gtest/gtest.h>
#include "TestItem.h"
#include "TestUser.h"
#include "TestPage.h"
#include <future>
#include <thread>
#include "drogon/drogon_test.h"

int main(int argc, char **argv) {
    // Initialize Google Test first
    ::testing::InitGoogleTest(&argc, argv);

    // Initialize the Drogon application
    drogon::app().loadConfigFile("./config.json");
    //
    auto promise = std::make_shared<std::promise<void>>();
    auto future = promise->get_future();

    // Start the main loop on another thread
    std::jthread appThread([promise]() {
        // Queue the promise to be fulfilled after starting the loop
        drogon::app().getLoop()->queueInLoop([promise]() {
            promise->set_value();
        });

        // Run the event loop
        drogon::app().run();
    });

    // Wait until the event loop has started
    future.get();

    // Run the tests
    const int testResult = RUN_ALL_TESTS();

    // After tests complete, shutdown the server
    drogon::app().getLoop()->queueInLoop([]() {
        drogon::app().quit();
    });

    // Wait for the application thread to finish
    appThread.join();

    return testResult;
}
