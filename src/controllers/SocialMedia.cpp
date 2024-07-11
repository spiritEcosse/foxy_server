#include "SocialMedia.h"
#include "ItemModel.h"
#include "MediaModel.h"
#include "QuerySet.h"
#include "TwitterClient.h"
#include <memory>
#include <future>
#include <vector>
#include <sentry.h>
#include "fmt/format.h"

using namespace api::v1;

void SocialMedia::handleRow(const auto &row) {
    auto &twitterClient = TwitterClient::getInstance();

    std::string title = row[0].template as<std::string>();
    auto itemId = row[1].template as<int>();
    auto slug = row[2].template as<std::string>();
    auto mediaList = row[3].template as<Json::Value>();
    std::cout << title << mediaList << std::endl;
    std::vector<FileTransferInfo> mediaUrls = {};
    std::for_each(mediaList.begin(), mediaList.end(), [&mediaUrls](const auto &media) {
        mediaUrls.emplace_back(media.asString(), media.asString().substr(media.asString().find_last_of('/') + 1));
    });
    Tweet tweet(title, std::move(mediaUrls), std::move(slug));
    twitterClient.postTweet(tweet);

    SocialMediaModel item(std::string("twitter"), tweet.tweetId, itemId);
    std::string query = SocialMediaModel().sqlInsert(item);
    auto dbClient = drogon::app().getDbClient("default_not_fast");
    dbClient->execSqlAsync(
        query,
        [](const drogon::orm::Result &r) {
            std::cout << "Inserted " << r.affectedRows() << " rows." << std::endl;
        },
        [](const drogon::orm::DrogonDbException &e) {
            LOG_ERROR << e.base().what();
            sentry_capture_event(sentry_value_new_message_event(
                /*   level */ SENTRY_LEVEL_ERROR,
                /*  logger */ "handleRow",
                /* message */ e.base().what()));
        });
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

    auto callbackPtr = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    auto dbClient = drogon::app().getFastDbClient("default");
    QuerySet qs(ItemModel::tableName, 1, "items", false, true);  // remove 1 and set all
    qs.only(ItemModel::Field::title, ItemModel::Field::id, ItemModel::Field::slug)
        .join(MediaModel())
        .left_join(SocialMediaModel())
        .filter(ItemModel::Field::id.getFullFieldName(), std::string("93"))  // remove
        .group_by(ItemModel::Field::id)
        .filter(SocialMediaModel::Field::id.getFullFieldName(), std::string("NULL"), false, std::string("IS"))
        .functions(Function(fmt::format(", json_agg(format_src(media.src, '{}')) AS media_list", app_cloud_name)));

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
