#include "Tweet.h"
#include <ranges>

namespace api::v1 {
    Tweet::Tweet(const int itemId,
                 const std::string_view& title,
                 const std::string_view& slug,
                 const std::string_view& description,
                 const std::vector<SharedFileTransferInfo>& media,
                 const Json::Value& tags) : SocialMediaType(itemId, title, slug, description, cutMedia(media), tags) {
        // formating title
        this->title = truncateTitle(fmt::format("{} {} {}", title, itemUrl, tagsToString()));
    }

    std::string Tweet::toJson() const {
        Json::Value jsonObj;
        jsonObj["text"] = title;
        Json::Value mediaArray = Json::arrayValue;
        //must be for_each, because of: no member named 'push_back' in 'Json::Value'
        std::ranges::for_each(concatVectors(videos, images), [&mediaArray](const SharedFileTransferInfo& info) {
            mediaArray.append(info->getExternalId());
        });

        jsonObj["media"] = Json::objectValue;
        jsonObj["media"]["media_ids"] = mediaArray;

        Json::StreamWriterBuilder builder;
        builder["commentStyle"] = "None";
        builder["indentation"] = "";  // Compact JSON
        return writeString(builder, jsonObj);
    }

    bool Tweet::post() {
        return client->uploadMedia(this) && SocialMediaType::post();
    }

};
