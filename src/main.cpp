#include "drogon/drogon.h"
#include "env.h"
#include <sentry.h>
#include <fmt/format.h>

using namespace drogon;

int main() {
    std::string env;

    if(!getenv("ENV", env)) {
        throw std::invalid_argument("ENV is not set");
    }

    std::string sentry_dsn;

    if(env != "dev") {
        if(!getenv("SENTRY_DSN", sentry_dsn)) {
            throw std::invalid_argument("SENTRY_DSN is not set");
        }

        sentry_options_t *options = sentry_options_new();
        sentry_options_set_dsn(options, sentry_dsn.c_str());
        // This is also the default-path. For further information and recommendations:
        // https://docs.sentry.io/platforms/native/configuration/options/#database-path
        //        sentry_options_set_database_path(options, ".sentry-native");
        sentry_options_set_handler_path(
            options,
            fmt::format("{}/_deps/sentry-build/crashpad_build/handler/crashpad_handler", CMAKE_BINARY_DIR).c_str());
        sentry_options_set_release(options, "faithfishart-server@0.0.1");
        sentry_options_set_debug(options, 1);
        sentry_init(options);
    }

    std::string config_app_path;
    if(!getenv("CONFIG_APP_PATH", config_app_path)) {
        throw std::invalid_argument("CONFIG_APP_PATH is not set");
    }

    std::string foxy_client;

    if(!getenv("FOXY_CLIENT", foxy_client)) {
        throw std::invalid_argument("FOXY_CLIENT is not set");
    }

    if(std::string app_cloud_name; !getenv("APP_CLOUD_NAME", app_cloud_name)) {
        throw std::invalid_argument("APP_CLOUD_NAME is not set");
    }

    std::string foxy_admin;
    if(!getenv("FOXY_ADMIN", foxy_admin)) {
        throw std::invalid_argument("FOXY_ADMIN is not set");
    }
    drogon::app().loadConfigFile(config_app_path);
    app().registerHandler("/", [](const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback) {
        Json::Value json;
        json["result"] = "ok";
        json["message"] = "hello,world!";
        auto resp = HttpResponse::newHttpJsonResponse(json);
        auto callbackPtr = std::make_shared<std::function<void(const HttpResponsePtr &)>>(std::move(callback));
        (*callbackPtr)(resp);
    });
    app().registerHandler("/sentry",
                          [](const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback) {
                              sentry_capture_event(sentry_value_new_message_event(
                                  /*   level */ SENTRY_LEVEL_INFO,
                                  /*  logger */ "custom",
                                  /* message */ "It works!"));
                              Json::Value json;
                              json["result"] = "ok";
                              json["message"] = "sentry, checked! (SENTRY_LEVEL_INFO, custom, It works!)";
                              auto resp = HttpResponse::newHttpJsonResponse(json);
                              auto callbackPtr =
                                  std::make_shared<std::function<void(const HttpResponsePtr &)>>(std::move(callback));
                              (*callbackPtr)(resp);
                          });
    app().registerHandler("/test?username={name}",
                          []([[maybe_unused]] const HttpRequestPtr &req,
                             std::function<void(const HttpResponsePtr &)> &&callback,
                             const std::string &name) {
                              Json::Value json;
                              json["result"] = "ok";
                              json["message"] = std::string("hello,") + name;
                              auto resp = HttpResponse::newHttpJsonResponse(json);
                              auto callbackPtr =
                                  std::make_shared<std::function<void(const HttpResponsePtr &)>>(std::move(callback));
                              (*callbackPtr)(resp);
                          });
    app().registerPostHandlingAdvice([foxy_client, foxy_admin]([[maybe_unused]] const drogon::HttpRequestPtr &req,
                                                               const drogon::HttpResponsePtr &resp) {
        auto origin = req->getHeader("Origin");
        if(origin == foxy_client || origin == foxy_admin) {
            resp->addHeader("Access-Control-Allow-Origin", origin);
        }
    });
    app().setThreadNum(std::thread::hardware_concurrency() + 2);
    std::string http_port;
    if(!getenv("FOXY_HTTP_PORT", http_port)) {
        throw std::invalid_argument("FOXY_HTTP_PORT is not set");
    }
    std::string host = env == "dev" ? "127.0.0.1" : "0.0.0.0";
    app().addListener(host, static_cast<uint16_t>(std::stoi(http_port))).run();

    if(env != "dev") {
        sentry_close();
    }
}
