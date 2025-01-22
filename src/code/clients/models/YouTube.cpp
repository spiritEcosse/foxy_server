#include "YouTube.h"
#include <execution>

namespace api::v1 {
    YouTube::YouTube(const int itemId,
                     const std::string_view& title,
                     const std::string_view& slug,
                     const std::string_view& description,
                     const std::vector<SharedFileTransferInfo>& media,
                     const Json::Value& tags) : SocialMediaType(itemId, title, slug, description, media, tags) {
        // formating description
        this->description = truncateDescription(fmt::format("{} {}", description, itemUrl));
    };

    YouTube::YouTube(const int itemId,
                     const std::string_view& title,
                     const std::string_view& slug,
                     const std::string_view& description,
                     const SharedFileTransferInfo& video,
                     const std::vector<std::string>& tags) :
        SocialMediaType(itemId, title, slug, description, tags), video(video) {};

    bool YouTube::post() {
        return postVideos();
    }

    bool YouTube::postVideos() {
        auto postingVideos = [this]() {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::vector<std::jthread> threads;
            std::atomic allSuccess{true};  // Track success across threads

            std::ranges::for_each(videos, [this, &threads, &allSuccess](const auto& videoItem) {
                threads.emplace_back([this, &videoItem, &allSuccess]() {
                    // Safely set allSuccess to false if any postVideo fails
                    if(!YouTube(itemId, title, slug, description, videoItem, tags).postVideo()) {
                        allSuccess.store(false, std::memory_order_relaxed);  // Avoid copying
                    }
                });
            });

            for(auto& thread: threads)
                thread.join();  // Wait for all threads to finish

            return allSuccess.load(std::memory_order_relaxed);  // Return final status
        };

        return postingVideos();
    }

    bool YouTube::postVideo() {
        return client->uploadVideo(this) && SocialMediaType::saveToDb();
    }

    std::string YouTube::toJson() const {
        Json::Value snippet;
        snippet["title"] = title;
        snippet["description"] = description;
        snippet["categoryId"] = 26;

        Json::Value tagsJson(Json::arrayValue);
        for(const auto& tag: tags) {
            tagsJson.append(tag);
        }
        snippet["tags"] = tagsJson;

        Json::Value status;
        status["privacyStatus"] = "private";
        // status["privacyStatus"] = "public";
        Json::Value mediaSource;
        mediaSource["snippet"] = snippet;
        mediaSource["status"] = status;

        Json::StreamWriterBuilder builder;
        builder["commentStyle"] = "None";
        builder["indentation"] = "";
        return writeString(builder, mediaSource);
    }
}
