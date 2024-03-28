#include "drogon/drogon.h"
#include "src/utils/env.h"

using namespace drogon;

int main() {
    std::string config_app_path;
    if(!getenv("CONFIG_APP_PATH", config_app_path)) {
        throw std::invalid_argument("CONFIG_APP_PATH is not set");
    }

    std::string env;

    if(!getenv("ENV", env)) {
        throw std::invalid_argument("ENV is not set");
    }
    std::string foxy_client;

    if(!getenv("FOXY_CLIENT", foxy_client)) {
        throw std::invalid_argument("FOXY_CLIENT is not set");
    }

    if(std::string app_cloud_name; !getenv("APP_CLOUD_NAME", app_cloud_name)) {
        throw std::invalid_argument("APP_CLOUD_NAME is not set");
    }

    std::string foxy_admin;
    if (!getenv("FOXY_ADMIN", foxy_admin)) {
        throw std::invalid_argument("FOXY_ADMIN is not set");
    }
    drogon::app().loadConfigFile(config_app_path);
    app().registerHandler("/",
                          [](const HttpRequestPtr &req,
                             std::function<void(const HttpResponsePtr &)> &&callback) {
                              Json::Value json;
                              json["result"] = "ok";
                              json["message"] = "hello,world!";
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
    app().registerPostHandlingAdvice(
        [foxy_client, foxy_admin]([[maybe_unused]] const drogon::HttpRequestPtr &req, const drogon::HttpResponsePtr &resp) {
            auto origin = req->getHeader("Origin");
            if (origin == foxy_client || origin == foxy_admin) {
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
}
