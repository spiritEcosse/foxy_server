#include "SocialMedia.h"
#include "ItemModel.h"
#include "MediaModel.h"
#include "QuerySet.h"
#include "TwitterClient.h"
#include <memory>
#include <future>
#include <vector>
#include "fmt/format.h"
#include "sentryHelper.h"
#include "Request.h"

using namespace api::v1;

void SocialMedia::handleRow(const auto &row) {
    auto &twitterClient = TwitterClient::getInstance();

    std::string title = row[0].template as<std::string>();
    auto itemId = row[1].template as<int>();
    auto slug = row[2].template as<std::string>();
    auto mediaList = row[3].template as<Json::Value>();
    auto tagsJson = row[4].template as<Json::Value>();
    // Convert tagsJson to std::vector<std::string>
    std::vector<std::string> tags;
    if(tagsJson.isArray()) {
        tags.reserve(tagsJson.size());
        for(const auto &tag: tagsJson) {
            tags.push_back(tag.asString());
        }
    }
    std::vector<FileTransferInfo> mediaUrls = {};
    std::ranges::for_each(mediaList, [&mediaUrls](const Json::Value &media) {
        std::string mediaUrl = media.asString();
        std::string fileName = mediaUrl.substr(mediaUrl.find_last_of('/') + 1);
        mediaUrls.emplace_back(fmt::format("{}?twic=v1/cover=4000", mediaUrl), fileName);
    });
    Tweet tweet(title, std::move(mediaUrls), std::move(slug), std::move(tags));
    twitterClient.postTweet(tweet);

    if(!tweet.tweetId.empty()) {
        SocialMediaModel item(std::string("Twitter"), tweet.tweetId, itemId);
        std::string query = SocialMediaModel().sqlInsert(item);
        auto dbClient = drogon::app().getDbClient("default_not_fast");
        dbClient->execSqlAsync(
            query,
            [](const drogon::orm::Result &r) {
                std::cout << "Inserted " << r.affectedRows() << " rows." << std::endl;
            },
            [](const drogon::orm::DrogonDbException &e) {
                std::string error = e.base().what();
                sentryHelper(error, "handleRow");
            });
    }
}

void SocialMedia::handleSqlResultPublish(const drogon::orm::Result &r) {
    std::vector<std::future<void>> futures;

    for(const auto &row: r) {
        // Wrap the call to handleRow in a lambda to ensure correct argument passing
        auto fut = std::async(std::launch::async, [this, row]() {
            this->handleRow(row);
        });
        futures.push_back(std::move(fut));
    }

    // Wait for all tasks to complete
    for(auto &fut: futures) {
        fut.get();
    }
}

void SocialMedia::publish(const drogon::HttpRequestPtr &req,
                          std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    std::string app_cloud_name;
    getenv("APP_CLOUD_NAME", app_cloud_name);
    std::string app_bucket_host;
    getenv("APP_BUCKET_HOST", app_bucket_host);
    int limit = getInt(req->getParameter("limit"), 1);

    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    auto dbClient = drogon::app().getFastDbClient("default");
    QuerySet qs(ItemModel::tableName, limit, "items", false);
    qs.only(ItemModel::Field::title, BaseModel<ItemModel>::Field::id, ItemModel::Field::slug)
        .join(MediaModel())
        .left_join(SocialMediaModel())
        .group_by(BaseModel<ItemModel>::Field::id)
        .filter(BaseModel<SocialMediaModel>::Field::id.getFullFieldName(),
                std::string("NULL"),
                false,
                std::string("IS"))
        .functions(Function(fmt::format(", json_agg("
                                        "CASE "
                                        "WHEN {0}::text LIKE 'video' THEN format_src({1}, '{3}') "
                                        "ELSE format_src({1}, '{2}') "
                                        "END "
                                        "ORDER BY CASE "
                                        "WHEN {0}::text LIKE 'video' THEN 1 "
                                        "ELSE 2 "
                                        "END ASC) AS media_list",
                                        MediaModel::Field::type.getFullFieldName(),
                                        MediaModel::Field::src.getFullFieldName(),
                                        app_cloud_name,
                                        app_bucket_host)))
        .functions(Function(fmt::format(" to_json(tags) AS tags_json")));

    executeSqlQuery(callbackPtr,
                    qs.buildSelect(),
                    [this](const drogon::orm::Result &r,
                           const std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> &_callbackPtr) {
                        Json::Value jsonResponse;
                        jsonResponse["result"] =
                            fmt::format("You are going to publish the count of {} items.", r.size());
                        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
                        resp->setStatusCode(drogon::HttpStatusCode::k200OK);
                        (*_callbackPtr)(resp);
                        this->handleSqlResultPublish(r);
                    });
}
