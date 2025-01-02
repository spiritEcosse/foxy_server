#include <gtest/gtest.h>
#include "TestItem.h"
#include "TestUser.h"
#include "TestPage.h"
#include "TestShippingProfile.h"
#include "TestShippingRate.h"
#include "TestReview.h"
#include "TestBasketItem.h"
#include "TestAddress.h"
#include "TestMedia.h"
#include "TestBasket.h"
#include "TestCountry.h"
#include "TestSocialMedia.h"
#include "TestOrder.h"

#include <future>
#include <thread>
#include "drogon/drogon_test.h"

static std::jthread appThread;
static std::atomic<bool> appRunning{false};  // Atomic flag for thread safety

class DrogonTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        if(!appRunning.exchange(true)) {  // Atomically check and set
            drogon::app().loadConfigFile("./config.json");

            auto promise = std::make_shared<std::promise<void>>();
            auto future = promise->get_future();

            appThread = std::jthread([promise]() {
                drogon::app().getLoop()->queueInLoop([promise]() {
                    promise->set_value();
                });
                drogon::app().run();
            });

            future.get();  // Wait for the app to start
        }
    }

    void TearDown() override {
        if(appRunning.exchange(false)) {  // Atomically check and set
            drogon::app().getLoop()->queueInLoop([]() {
                drogon::app().quit();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            });
            appThread.join();
        }
    }
};

::testing::Environment *const env = ::testing::AddGlobalTestEnvironment(new DrogonTestEnvironment());

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    // Run all tests
    return RUN_ALL_TESTS();
}
