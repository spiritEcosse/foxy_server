#include "Pin.h"
#include "env.h"

namespace api::v1 {
    Pin::Pin(const int itemId,
             const std::string_view& title,
             const std::string_view& slug,
             const std::string_view& description,
             const std::vector<SharedFileTransferInfo>& media,
             const Json::Value& tags) : SocialMediaType(itemId, title, slug, description, media, tags) {
        // formating description
        this->description = truncateDescription(fmt::format("{} {}", description, tagsToString()));
    };

    bool Pin::post() {
        return SocialMediaType::post();
    }

    std::string Pin::toJson() {
        // {
        //     "title": "My Pin",
        //     "description": "Pin Description",
        //     "board_id": "876513214917113468",
        //     "media_source": {
        //         "source_type": "multiple_image_base64",
        //         "items": [
        //             {
        //                 "base64": {
        //                     "data": "base64EncodedImageString1...",
        //                     "content_type": "image/jpeg"
        //                 },
        //                 "link": "https://example.com/image1"
        //             },
        //             {
        //                 "base64": {
        //                     "data": "base64EncodedImageString2...",
        //                     "content_type": "image/png"
        //                 },
        //                 "link": "https://example.com/image2"
        //             }
        //         ]
        //     }
        // }
        Json::Value jsonObj;
        jsonObj["title"] = title;
        jsonObj["description"] = description;
        jsonObj["board_id"] = std::string(boardId);

        // Create media_source object
        Json::Value mediaSource;
        mediaSource["source_type"] = "multiple_image_base64";

        // Create items array for multiple images !!!!!!!!!!!!!!!!!!!!!!!!!!!
        Json::Value items = Json::arrayValue;
        std::ranges::for_each(media | std::views::filter([](const SharedFileTransferInfo& info) {
                                  return !info->isVideo();
                              }),
                              [&items](const SharedFileTransferInfo& info) {
                                  Json::Value item;
                                  item["data"] = info->getBase64Response();
                                  item["content_type"] = info->getContentType();
                                  item["link"] = info->getUrl();
                                  items.append(item);
                              });

        // Add items array to media_source
        mediaSource["items"] = items;

        // Add media_source to root object
        jsonObj["media_source"] = mediaSource;

        // Build the JSON string
        Json::StreamWriterBuilder builder;
        builder["commentStyle"] = "None";
        builder["indentation"] = "";  // Compact JSON
        return writeString(builder, jsonObj);
    }
}
