#include "drogon/drogon.h"
#include "config.h"
#if defined(SENTRY_DSN)
#include <sentry.h>
#endif
#include <fmt/format.h>
#include "sentryHelper.h"
#include <iostream>

using namespace drogon;

int main() {
    try {
#if defined(SENTRY_DSN)
        sentry_options_t *options = sentry_options_new();
        sentry_options_set_dsn(options, SENTRY_DSN);
        sentry_options_set_handler_path(
            options,
            fmt::format("{}/_deps/sentry-build/crashpad_build/handler/crashpad_handler", CMAKE_BINARY_DIR).c_str());
        sentry_options_set_release(options, "faithfishart-server@0.0.1");
        sentry_options_set_debug(options, 1);
        sentry_init(options);
#endif

        const auto pgDb = api::v1::getEnv("PG_DB", "foxy");
        const auto pgUser = api::v1::getEnv("PG_USER", "foxy");
        app().createDbClient("postgresql", "/var/run/postgresql", 5432, pgDb, pgUser, "", 1, "", "default", true);
        app().createDbClient("postgresql",
                             "/var/run/postgresql",
                             5432,
                             pgDb,
                             pgUser,
                             "",
                             1,
                             "",
                             "default_not_fast",
                             false);
        app().registerHandler(
            "/",
            []([[maybe_unused]] const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback) {
                Json::Value json;
                json["result"] = "ok";
                json["message"] = "hello,world!";
                auto resp = HttpResponse::newHttpJsonResponse(json);
                auto callbackPtr = std::make_shared<std::function<void(const HttpResponsePtr &)>>(std::move(callback));
                (*callbackPtr)(resp);
            });
#if defined(SENTRY_DSN)
        app().registerHandler("/sentry",
                              [](const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback) {
                                  sentryHelper("It works!", "custom");
                                  Json::Value json;
                                  json["result"] = "ok";
                                  json["message"] = "sentry, checked! (SENTRY_LEVEL_INFO, custom, It works!)";
                                  auto resp = HttpResponse::newHttpJsonResponse(json);
                                  auto callbackPtr = std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                                      std::move(callback));
                                  (*callbackPtr)(resp);
                              });
#endif

        app().registerHandler("/test?username={name}",
                              []([[maybe_unused]] const HttpRequestPtr &req,
                                 std::function<void(const HttpResponsePtr &)> &&callback,
                                 const std::string &name) {
                                  Json::Value json;
                                  json["result"] = "ok";
                                  json["message"] = std::string("hello,") + name;
                                  auto resp = HttpResponse::newHttpJsonResponse(json);
                                  auto callbackPtr = std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                                      std::move(callback));
                                  (*callbackPtr)(resp);
                              });
        app().registerPostHandlingAdvice([]([[maybe_unused]] const HttpRequestPtr &req, const HttpResponsePtr &resp) {
            auto origin = req->getHeader("Origin");
            const auto foxyClient = api::v1::getEnv("FOXY_CLIENT", "");
            const auto foxyAdmin = api::v1::getEnv("FOXY_ADMIN", "");
            if(origin == foxyClient || origin == foxyAdmin) {
                resp->addHeader("Access-Control-Allow-Origin", origin);
            }
        });
        app().setThreadNum(std::thread::hardware_concurrency() + 2);
        app().addListener("0.0.0.0", 8080);
        app().run();
#if defined(SENTRY_DSN)
        sentry_close();
#endif

    } catch(...) {
        std::cerr << "Unhandled exception in main" << std::endl;
    }
    return 0;
}
