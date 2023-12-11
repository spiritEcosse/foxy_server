#include <drogon/drogon.h>

using namespace drogon;

bool getenv(const char *name, std::string &env)
{
    const char *ret = getenv(name);
    if (ret) env = std::string(ret);
    return !!ret;
}

int main()
{
    std::string config_app_path;
    if (!getenv("CONFIG_APP_PATH", config_app_path)){
        throw std::invalid_argument("CONFIG_APP_PATH is not set");
    }

    std::string env;

    if (!getenv("ENV", env)){
        throw std::invalid_argument("ENV is not set");
    }
    std::string foxy_client;

    if (!getenv("FOXY_CLIENT", foxy_client)){
        throw std::invalid_argument("FOXY_CLIENT is not set");
    }

    drogon::app().loadConfigFile(config_app_path);
    app().registerHandler("/test?username={name}",
                          []([[maybe_unused]] const HttpRequestPtr& req,
                             std::function<void (const HttpResponsePtr &)> &&callback,
                             const std::string &name)
                          {
                              Json::Value json;
                              json["result"]="ok";
                              json["message"]=std::string("hello,")+name;
                              auto resp=HttpResponse::newHttpJsonResponse(json);
                              auto callbackPtr = std::make_shared<std::function<void(const HttpResponsePtr &)>>(std::move(callback));
                              (*callbackPtr)(resp);
                          });
    app().registerPostHandlingAdvice(
        [foxy_client]([[maybe_unused]] const drogon::HttpRequestPtr &req, const drogon::HttpResponsePtr &resp) {
            resp->addHeader("Access-Control-Allow-Origin", foxy_client);
        });
    app().setThreadNum(std::thread::hardware_concurrency() + 2);
    std::string http_port;
    if (!getenv("FOXY_HTTP_PORT", http_port)){
        throw std::invalid_argument("FOXY_HTTP_PORT is not set");
    }
    std::string host = env == "dev" ? "127.0.0.1" : "0.0.0.0";
    app().addListener(host, static_cast<uint16_t>(std::stoi(http_port))).run();
    std::cout << "server started" << std::endl;
}
