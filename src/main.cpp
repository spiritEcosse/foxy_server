#include "drogon/drogon.h"
#include "env.h"
#include <sentry.h>
#include <fmt/format.h>
#include "sentryHelper.h"
#include "backward-cpp/backward.hpp"
#include <csignal>
#include <iostream>

using namespace drogon;

[[noreturn]] void handleSignal(int signal) {
    std::cerr << "Caught signal " << signal << std::endl;
    // Add your cleanup code or logging here
    // Capture and print the stack trace
    backward::StackTrace st;
    st.load_here(32);  // You can adjust the number of frames captured
    backward::Printer p;
    p.print(st, std::cerr);  // Print the stack trace to stderr
    std::exit(signal);
}

int main() {
    std::signal(SIGTRAP, handleSignal);

    try {
        if(strcmp(ENVIRONMENT, "dev") != 0) {
            sentry_options_t *options = sentry_options_new();
            sentry_options_set_dsn(options, SENTRY_DSN);
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

        app().loadConfigFile(CONFIG_APP_PATH);
        app().registerHandler("/",
                              [](const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback) {
                                  Json::Value json;
                                  json["result"] = "ok";
                                  json["message"] = "hello,world!";
                                  auto resp = HttpResponse::newHttpJsonResponse(json);
                                  auto callbackPtr = std::make_shared<std::function<void(const HttpResponsePtr &)>>(
                                      std::move(callback));
                                  (*callbackPtr)(resp);
                              });
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
            if(origin == FOXY_CLIENT || origin == FOXY_ADMIN) {
                resp->addHeader("Access-Control-Allow-Origin", origin);
            }
        });
        app().setThreadNum(std::thread::hardware_concurrency() + 2);
        const std::string host = (strcmp(ENVIRONMENT, "dev") == 0) ? "127.0.0.1" : "0.0.0.0";
        app().addListener(host, static_cast<uint16_t>(std::stoi(FOXY_HTTP_PORT))).run();

        if(strcmp(ENVIRONMENT, "dev") != 0) {
            sentry_close();
        }
    } catch(...) {
        backward::StackTrace st;
        st.load_here(32);  // Capture the stack trace with a maximum of 32 frames
        backward::Printer p;
        p.print(st, std::cerr);  // Print the stack trace to stderr
    }
    return 0;
}
