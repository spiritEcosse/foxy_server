#include "Pin.h"
#include <execution>

namespace api::v1 {
    Pin::Pin(const int itemId,
             const std::string_view& title,
             const std::string_view& slug,
             const std::string_view& description,
             const std::vector<SharedFileTransferInfo>& media,
             const Json::Value& tags) : SocialMediaType(itemId, title, slug, description, media, tags) {
        // formating description
        images = cutMedia(images);
        this->description = truncateDescription(fmt::format("{} {} {}", description, itemUrl, tagsToString()));
    };

    Pin::Pin(const int itemId,
             const std::string_view& title,
             const std::string_view& slug,
             const std::string_view& description,
             const SharedFileTransferInfo& coverImage,
             const SharedFileTransferInfo& video,
             const std::vector<std::string>& tags) :
        SocialMediaType(itemId, title, slug, description, tags), coverImage(coverImage), video(video) {};

    bool Pin::post() {
        std::future<bool> postVideoFuture = std::async(std::launch::async, [this]() {
            return postVideos();
        });

        std::future<bool> postImagesFuture = std::async(std::launch::async, [this]() {
            return !images.empty() && SocialMediaType::post();
        });

        const bool postImagesResult = postImagesFuture.get();
        const bool postVideoResult = postVideoFuture.get();
        return postImagesResult && postVideoResult;
    }

    bool Pin::postVideos() {
        auto postingVideos = [this]() {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::vector<std::jthread> threads;
            std::ranges::for_each(videos, [this, &threads](const auto& videoItem) {
                threads.emplace_back([this, &videoItem]() {
                    return Pin(itemId, title, slug, description, images[0], videoItem, tags).postVideo();
                });
            });

            for(auto& thread: threads)
                thread.join();

            return true;
        };

        return !videos.empty() && !images.empty() && client->uploadVideos(this) && postingVideos();
    }

    bool Pin::postVideo() {
        return video && coverImage && client->post(this, toJsonVideo()) && SocialMediaType::saveToDb();
    }

    std::string Pin::toJsonInternal(const Json::Value& mediaSource) const {
        Json::Value jsonObj;
        jsonObj["title"] = title;
        jsonObj["description"] = description;
        jsonObj["board_id"] = std::string(boardId);
        jsonObj["media_source"] = mediaSource;

        Json::StreamWriterBuilder builder;
        builder["commentStyle"] = "None";
        builder["indentation"] = "";
        return writeString(builder, jsonObj);
    }

    std::string Pin::toJsonVideo() const {
        Json::Value mediaSource;
        mediaSource["source_type"] = "video_id";
        mediaSource["cover_image_content_type"] = coverImage->getContentType();
        mediaSource["cover_image_data"] = coverImage->getBase64ContentOfFile();
        mediaSource["media_id"] = video->getExternalId<Pin>();
        return toJsonInternal(mediaSource);
    }

    std::string Pin::toJson() const {
        Json::Value mediaSource;
        mediaSource["source_type"] = "multiple_image_base64";

        Json::Value items = Json::arrayValue;
        std::ranges::for_each(images, [&items, this](const auto& info) {
            Json::Value item;
            item["data"] = info->getBase64ContentOfFile();
            item["content_type"] = info->getContentType();
            item["link"] = itemUrl;
            items.append(item);
        });

        mediaSource["items"] = items;
        return toJsonInternal(mediaSource);
    }
}
