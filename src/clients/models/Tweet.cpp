#include "Tweet.h"

namespace api::v1 {
    Tweet::Tweet(const int itemId,
                 const std::string_view& title,
                 const std::string_view& slug,
                 const std::string_view& description,
                 const std::vector<SharedFileTransferInfo>& media,
                 const Json::Value& tags) : SocialMediaType(itemId, title, slug, description, media, tags) {
        // formating title
        this->title = truncateTitle(fmt::format("{} {} {}", title, itemUrl, tagsToString()));
    }

    std::string Tweet::toJson() {
        Json::Value jsonObj;
        jsonObj["text"] = title;
        Json::Value mediaArray = Json::arrayValue;
        //must be for_each, because of: no member named 'push_back' in 'Json::Value'
        std::ranges::for_each(media, [&mediaArray](const SharedFileTransferInfo& info) {
            mediaArray.append(info->getExternalId());
        });

        jsonObj["media"] = Json::objectValue;
        jsonObj["media"]["media_ids"] = mediaArray;

        Json::StreamWriterBuilder builder;
        builder["commentStyle"] = "None";
        builder["indentation"] = "";  // Compact JSON
        return writeString(builder, jsonObj);
    }

    void Tweet::post() {
        if(!client->uploadMedia(this)) {
            return;
        }
        std::cout << "TwitterClient::uploadMedia successfully" << std::endl;
        SocialMediaType::post();
    }

};
