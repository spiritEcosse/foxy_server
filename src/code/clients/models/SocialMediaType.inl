#pragma once
#include "utils/db/StringUtils.h"
#include "clients/PinterestClient.h"  // must be because of it : ClientType::clientName
#include "models/SocialMediaModel.h"
#include "clients/TwitterClient.h"  // must be because of it : ClientType::clientName
#include "clients/YouTubeClient.h"  // must be because of it : ClientType::clientName
#include "sentry_catcher/sentryHelper.h"
#include "utils/config.h"
#include <ranges>

namespace api::v1 {

    template<typename ClientType, typename PostType>
    std::string SocialMediaType<ClientType, PostType>::createItemUrl(const std::string_view &slug) {
        return fmt::format("{}/item/{}", getEnv("FOXY_CLIENT", ""), slug);
    }

    template<typename ClientType, typename PostType>
    std::string SocialMediaType<ClientType, PostType>::tagsToString() {
        const std::string &hashtags = fmt::format("{}", fmt::join(this->tags, " #"));
        return hashtags.empty() ? "" : "#" + hashtags;
    }

    template<typename ClientType, typename PostType>
    bool SocialMediaType<ClientType, PostType>::post() {
        return client->post(static_cast<PostType *>(this)) && saveToDb();
    }

    template<typename ClientType, typename PostType>
    bool SocialMediaType<ClientType, PostType>::saveToDb() {
        if(postId.empty())
            return false;
        const SocialMediaModel item(std::string(ClientType::clientName), postId, itemId);
        std::string query = SocialMediaModel().sqlInsert(item);
        auto dbClient = drogon::app().getFastDbClient("default");
        dbClient->execSqlAsync(
            query,
            [](const drogon::orm::Result &r) {
                std::cout << "Inserted " << r.affectedRows() << " rows." << std::endl;
            },
            [](const drogon::orm::DrogonDbException &e) {
                const std::string error = e.base().what();
                sentryHelper(error, "saveToDb");
            });
        return true;
    }

    template<typename ClientType, typename PostType>
    void SocialMediaType<ClientType, PostType>::createClient() {
        client = std::make_unique<ClientType>();
    }

    template<typename ClientType, typename PostType>
    std::string SocialMediaType<ClientType, PostType>::truncateDescription(const std::string_view &description) {
        std::string cleanDescription = removeHtmlTags(std::string(description));

        return truncateText(cleanDescription.find(INTRODUCTION_TEXT_POST) != std::string_view::npos
                                ? cleanDescription
                                : fmt::format("{} {}", INTRODUCTION_TEXT_POST, cleanDescription),
                            PostType::maxDescriptionSize);
    }

    template<typename ClientType, typename PostType>
    std::string SocialMediaType<ClientType, PostType>::truncateTitle(const std::string_view &title) {
        if(PostType::maxDescriptionSize == 0)
            return truncateText(fmt::format("{} {}", INTRODUCTION_TEXT_POST, title), PostType::maxTitleSize);
        return truncateText(title, PostType::maxTitleSize);
    }

    template<typename ClientType, typename PostType>
    bool SocialMediaType<ClientType, PostType>::isEqualPlatform(const Json::Value &platform) {
        return platform.asString() == ClientType::clientName;
    }

    template<typename ClientType, typename PostType>
    std::vector<std::string> SocialMediaType<ClientType, PostType>::extractTags(const Json::Value &tagsJson) {
        std::vector<std::string> tags;

        std::ranges::for_each(tagsJson, [&tags](const auto &tag) {
            std::string tagTitle = tag["title"].asString();
            if(std::ranges::any_of(tag["social_media"], isEqualPlatform))
                tags.emplace_back(tagTitle);
        });

        return tags;
    }

    template<typename ClientType, typename PostType>
    std::vector<SharedFileTransferInfo>
    SocialMediaType<ClientType, PostType>::cutMedia(const std::vector<SharedFileTransferInfo> &mediaOriginal) {
        std::vector<SharedFileTransferInfo> media;
        const size_t numItems = std::min(static_cast<size_t>(PostType::maxMediaItems), mediaOriginal.size());
        media.reserve(numItems);

        std::ranges::transform(mediaOriginal | std::ranges::views::take(numItems),
                               std::back_inserter(media),
                               [](const auto &mediaObj) {
                                   return mediaObj;
                               });
        return media;
    }

    template<typename ClientType, typename PostType>
    std::vector<SharedFileTransferInfo>
    SocialMediaType<ClientType, PostType>::getImages(const std::vector<SharedFileTransferInfo> &mediaOriginal) {
        // TODO: replace with ranges::to from c++23
        std::vector<SharedFileTransferInfo> medias;
        std::ranges::copy_if(mediaOriginal, std::back_inserter(medias), [](const auto &mediaItem) {
            return !mediaItem->isVideo();
        });
        return medias;
    }

    template<typename ClientType, typename PostType>
    std::vector<SharedFileTransferInfo>
    SocialMediaType<ClientType, PostType>::getVideos(const std::vector<SharedFileTransferInfo> &mediaOriginal) {
        // TODO: replace with ranges::to from c++23
        std::vector<SharedFileTransferInfo> medias;
        std::ranges::copy_if(mediaOriginal, std::back_inserter(medias), [](const auto &mediaItem) {
            return mediaItem->isVideo();
        });
        return medias;
    }
}
