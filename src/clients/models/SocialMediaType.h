#pragma once

#include "SocialMediaTypeImpl.h"
#include <string>
#include <vector>
#include <drogon/drogon.h>
#include "FileTransferInfo.h"

const std::string INTRODUCTION_TEXT_POST = "Explore #FaithFishArt: Discover and buy inspiring art. Follow for updates!";

namespace api::v1 {
    template<typename ClientType, typename PostType>
    class SocialMediaType : public SocialMediaTypeImpl {
    public:
        SocialMediaType(const int itemId,
                        const std::string_view& title,
                        const std::string_view& slug,
                        const std::string_view& description,
                        const std::vector<SharedFileTransferInfo>& media,
                        const Json::Value& tags) :
            SocialMediaTypeImpl(), title(truncateTitle(title)), slug(slug),
            description(truncateDescription(description)), itemUrl(createItemUrl(slug)), itemId(itemId),
            media(cutMedia(media)), tags(extractTags(tags)) {
            createClient();
        }

        virtual bool post();
        virtual bool saveToDb();
        std::unique_ptr<ClientType> client;
        std::string title;
        std::string slug;
        std::string description;
        std::string itemUrl;
        int itemId;
        std::vector<SharedFileTransferInfo> media;
        std::vector<std::string> tags;
        std::string postId;

    protected:
        static std::vector<std::string> extractTags(const Json::Value& tagsJson);
        static std::string truncateDescription(const std::string_view& description);
        static std::string truncateTitle(const std::string_view& title);
        static std::string createItemUrl(const std::string_view& slug);
        static std::vector<SharedFileTransferInfo> cutMedia(const std::vector<SharedFileTransferInfo>& mediaOriginal);
        std::string tagsToString();
        virtual std::string toJson() = 0;
        void createClient();
    };
}

#include "SocialMediaType.inl"
