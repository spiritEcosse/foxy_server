#include "drogon/drogon.h"
#include "utils/config.h"
#if defined(SENTRY_DSN)
#include <sentry.h>
#endif
#include <fmt/format.h>
#include "sentry_catcher/sentryHelper.h"
#include <iostream>

using namespace drogon;

namespace {
    void sendJson(std::function<void(const HttpResponsePtr &)> &&callback, Json::Value &&json) {
        auto resp = HttpResponse::newHttpJsonResponse(std::move(json));
        auto callbackPtr = std::make_shared<std::function<void(const HttpResponsePtr &)>>(std::move(callback));
        (*callbackPtr)(resp);
    }

    void rootHandler(const HttpRequestPtr &, std::function<void(const HttpResponsePtr &)> &&callback) {
        Json::Value json;
        json["result"] = "ok";
        json["message"] = "hello,world!";
        sendJson(std::move(callback), std::move(json));
    }

    void testHandler(const HttpRequestPtr &,
                     std::function<void(const HttpResponsePtr &)> &&callback,
                     const std::string &name) {
        Json::Value json;
        json["result"] = "ok";
        json["message"] = "hello," + name;
        sendJson(std::move(callback), std::move(json));
    }

#if defined(SENTRY_DSN)
    void sentryHandler(const HttpRequestPtr &, std::function<void(const HttpResponsePtr &)> &&callback) {
        sentryHelper("It works!", "custom");
        Json::Value json;
        json["result"] = "ok";
        json["message"] = "sentry, checked! (SENTRY_LEVEL_INFO, custom, It works!)";
        sendJson(std::move(callback), std::move(json));
    }

    void initSentry() {
        sentry_options_t *options = sentry_options_new();
        sentry_options_set_dsn(options, SENTRY_DSN);
        sentry_options_set_handler_path(
            options,
            fmt::format("{}/_deps/sentry-build/crashpad_build/handler/crashpad_handler", CMAKE_BINARY_DIR).c_str());
        sentry_options_set_release(options, "faithfishart-server@0.0.1");
        sentry_options_set_debug(options, 1);
        sentry_init(options);
    }
#endif

    void addCorsHeader(const HttpRequestPtr &req, const HttpResponsePtr &resp) {
        const auto origin = req->getHeader("Origin");
        const auto foxyClient = api::v1::getEnv("FOXY_CLIENT", "");
        const auto foxyAdmin = api::v1::getEnv("FOXY_ADMIN", "");
        if(origin == foxyClient || origin == foxyAdmin)
            resp->addHeader("Access-Control-Allow-Origin", origin);
    }

    void registerRoutes() {
        app().registerHandler("/", &rootHandler);
        app().registerHandler("/test?username={name}", &testHandler);
#if defined(SENTRY_DSN)
        app().registerHandler("/sentry", &sentryHandler);
#endif
        app().registerPostHandlingAdvice(&addCorsHeader);
    }

    void setupDb() {
        const auto pgDb = api::v1::getEnv("PG_DB", "foxy");
        const auto pgUser = api::v1::getEnv("PG_USER", "foxy");
        app().createDbClient("postgresql", "/var/run/postgresql", 5432, pgDb, pgUser, "", 1, "", "default", true);
    }
}  // namespace

int main() {
    try {
#if defined(SENTRY_DSN)
        initSentry();
#endif
        setupDb();
        registerRoutes();
        app().setClientMaxBodySize(20 * 1024 * 1024);
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
