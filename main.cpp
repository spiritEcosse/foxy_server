#include <drogon/drogon.h>

using namespace drogon;

int main()
{
    drogon::app().loadConfigFile("/Users/ihor/projects/foxy_server/config.json");
    app().registerHandler("/test?username={name}",
                          [](const HttpRequestPtr& req,
                             std::function<void (const HttpResponsePtr &)> &&callback,
                             const std::string &name)
                          {
                              Json::Value json;
                              json["result"]="ok";
                              json["message"]=std::string("hello,")+name;
                              auto resp=HttpResponse::newHttpJsonResponse(json);
                              callback(resp);
                          });
    app().registerPostHandlingAdvice(
        [](const drogon::HttpRequestPtr &req, const drogon::HttpResponsePtr &resp) {
            //LOG_DEBUG << "postHandling1";
            resp->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        });
    app().setThreadNum(16);
    app().addListener("127.0.0.1", 8848).run();
}
