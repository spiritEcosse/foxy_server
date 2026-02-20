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
#include "TestFinancialDetails.h"
#include "TestOrder.h"
#include "config.h"

#include <future>
#include <thread>
#include "drogon/drogon_test.h"

static std::jthread appThread;
static std::atomic<bool> appRunning{false};

class DrogonTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        if(!appRunning.exchange(true)) {
            const auto pgDb = api::v1::getEnv("PG_DB", "foxy");
            const auto pgUser = api::v1::getEnv("PG_USER", "foxy");
            drogon::app().createDbClient(
                "postgresql", "/var/run/postgresql", 5432, pgDb, pgUser, "", 1, "", "default", true);
            drogon::app().createDbClient(
                "postgresql", "/var/run/postgresql", 5432, pgDb, pgUser, "", 1, "", "default_not_fast", false);

            auto promise = std::make_shared<std::promise<void>>();
            auto future = promise->get_future();

            appThread = std::jthread([promise]() {
                drogon::app().getLoop()->queueInLoop([promise]() {
                    promise->set_value();
                });
                drogon::app().run();
            });

            future.get();
        }
    }

    void TearDown() override {
        if(appRunning.exchange(false)) {
            drogon::app().getLoop()->queueInLoop([]() {
                drogon::app().quit();
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            });
            appThread.join();
        }
    }
};

::testing::Environment *const env = ::testing::AddGlobalTestEnvironment(new DrogonTestEnvironment());

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
