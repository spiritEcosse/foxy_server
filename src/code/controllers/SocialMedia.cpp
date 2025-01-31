#include "SocialMedia.h"
#include "ItemModel.h"
#include "MediaModel.h"
#include "QuerySet.h"
#include <memory>
#include <future>
#include <vector>
#include "Request.h"
#include "TagModel.h"
#include "Pin.h"
#include "Tweet.h"
#include "MastodonClient.h"
#include <execution>

#include <string>

using namespace api::v1;

void SocialMedia::handleRow(const auto &row) const {
    std::string title = row[0].template as<std::string>();
    auto itemId = row[1].template as<int>();
    auto slug = row[2].template as<std::string>();
    auto description = row[3].template as<std::string>();
    auto media = row[4].template as<Json::Value>();
    auto netsJson = row[5].template as<Json::Value>();
    auto tags = row[6].template as<Json::Value>();
    std::cout << "id: " << itemId << std::endl;

    std::set<std::string, std::less<>> target = {std::string(TwitterClient::clientName),
                                                 std::string(PinterestClient::clientName)};

    std::vector<std::string> nets;
    for(const auto &net: netsJson) {
        nets.emplace_back(net.asString());
    }

    std::set<std::string, std::less<>> netsSet(nets.begin(), nets.end());

    // Calculate difference: elements in target that do NOT exist in netsSet
    std::set<std::string, std::less<>> diffNets;
    std::ranges::set_difference(target, netsSet, std::inserter(diffNets, diffNets.end()));

    if(diffNets.empty())
        return;
    auto clientDownloadMedia = MastodonClient(media);
    if(!clientDownloadMedia.downloadMedia())
        return;

    std::future<bool> tweetPost = std::async(std::launch::async, [&]() {
        return diffNets.contains(TwitterClient::clientName) &&
               Tweet(itemId, title, slug, "", clientDownloadMedia.media, tags).post();
    });
    // std::future<bool> pinPost = std::async(std::launch::async, [&]() {
    //     return diffNets.contains(PinterestClient::clientName) &&
    //            Pin(itemId, title, slug, description, clientDownloadMedia.media, tags).post();
    // });
    tweetPost.get();
    // pinPost.get();
}

void SocialMedia::handleSqlResultPublish(const drogon::orm::Result &r) const {
    // The error occurs because drogon::orm::ConstResultIterator does not support random access. The standard algorithm std::for_each with std::execution::par requires random-access iterators (like std::vector::iterator or std::array::iterator), but Drogon's ConstResultIterator is a bidirectional iterator, similar to those used in linked lists.
    // std::ranges::for_each(r, [this](const auto &row) {
    //     handleRow(row);
    // });

    if(r.empty())
        return;

    // Number of threads to use (can be std::thread::hardware_concurrency() for dynamic detection)
    // Divide tasks among threads
    std::vector<std::future<void>> futures;

    for(const auto &row: r) {
        // Launch each row as a separate task
        futures.push_back(std::async(std::launch::async, [this, row]() {
            handleRow(row);
        }));
    }

    // Wait for all tasks to complete
    for(auto &fut: futures) {
        fut.get();
    }
}

void SocialMedia::publish(const drogon::HttpRequestPtr &req,
                          std::function<void(const drogon::HttpResponsePtr &)> &&callback) const {
    const int limit = getInt(req->getParameter("limit"), 1);

    QuerySet qsTag(TagModel::tableName, 0, std::string("_tag"), false);
    qsTag.functions(Function(fmt::format("json_agg(json_build_object({}))", TagModel().fieldsJsonObject())))
        .filter(TagModel::Field::itemId.getFullFieldName(),
                fmt::format("{}", BaseModel<ItemModel>::Field::id.getFullFieldName()),
                false);

    QuerySet qsSocialMedia(SocialMediaModel::tableName, 0, std::string("_social_media"), false);
    qsSocialMedia.functions(Function(fmt::format("json_agg({})", SocialMediaModel::Field::title.getFullFieldName())))
        .filter(SocialMediaModel::Field::itemId.getFullFieldName(),
                fmt::format("{}", BaseModel<ItemModel>::Field::id.getFullFieldName()),
                false);

    const auto callbackPtr =
        std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    auto dbClient = drogon::app().getFastDbClient("default");  // TODO: remove this line after test
    QuerySet qs(ItemModel::tableName, limit, "items", false);
    qs.only(std::cref(ItemModel::Field::title),
            std::cref(BaseModel<ItemModel>::Field::id),
            std::cref(ItemModel::Field::slug),
            std::cref(ItemModel::Field::description))
        .join(MediaModel())
        .left_join(SocialMediaModel())
        .filter(BaseModel<SocialMediaModel>::Field::id.getFullFieldName(),
                std::string("NULL"),
                false,
                std::string("IS"))
        .group_by(std::cref(BaseModel<ItemModel>::Field::id))
        .functions(Function(fmt::format("json_agg("
                                        "json_build_object('type', {0}, 'url', CASE "
                                        "WHEN {0}::text LIKE 'video' THEN format_src({1}, '{3}') "
                                        "ELSE format_src({1}, '{2}') "
                                        "END, 'content_type', {4}) ORDER BY CASE "
                                        "WHEN {0}::text LIKE 'video' THEN 1 "
                                        "ELSE 2 "
                                        "END ASC) AS media_list",
                                        MediaModel::Field::type.getFullFieldName(),
                                        MediaModel::Field::src.getFullFieldName(),
                                        APP_CLOUD_NAME,
                                        APP_BUCKET_HOST,
                                        MediaModel::Field::contentType.getFullFieldName())))
        .functions(Function(fmt::format(R"( COALESCE(({}), '[]'::json) AS nets)", qsSocialMedia.buildSelect())))
        .functions(Function(fmt::format(R"( COALESCE(({}), '[]'::json) AS tags)", qsTag.buildSelect())));

    executeSqlQuery(callbackPtr,
                    qs.buildSelect(),
                    [this](const drogon::orm::Result &r,
                           const std::shared_ptr<std::function<void(const drogon::HttpResponsePtr &)>> &_callbackPtr) {
                        Json::Value jsonResponse;
                        jsonResponse["result"] =
                            fmt::format("You are going to publish the count of {} items.", r.size());
                        const auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(jsonResponse));
                        resp->setStatusCode(drogon::HttpStatusCode::k200OK);
                        (*_callbackPtr)(resp);
                        this->handleSqlResultPublish(r);
                    });
}
