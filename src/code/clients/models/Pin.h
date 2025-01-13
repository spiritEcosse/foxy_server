#pragma once

#include "SocialMediaType.h"
#include "env.h"

namespace api::v1 {
    class Pin final : public SocialMediaType<PinterestClient, Pin> {
        static constexpr std::string_view boardId = PINTEREST_BOARD_ID;
        SharedFileTransferInfo coverImage;
        SharedFileTransferInfo video;
        [[nodiscard]] bool postVideo();
        [[nodiscard]] bool postVideos();
        std::string toJsonInternal(const Json::Value& mediaSource) const;

    public:
        static constexpr int maxDescriptionSize = 800;
        static constexpr int maxTitleSize = 100;
        static constexpr int maxMediaItems = 5;

        Pin(int itemId,
            const std::string_view& titleOriginal,
            const std::string_view& slug,
            const std::string_view& description,
            const std::vector<SharedFileTransferInfo>& media,
            const Json::Value& tags);

        Pin(int itemId,
            const std::string_view& title,
            const std::string_view& slug,
            const std::string_view& description,
            const SharedFileTransferInfo& coverImage,
            const SharedFileTransferInfo& video,
            const std::vector<std::string>& tags);

        std::string toJson() const override;
        std::string toJsonVideo() const;
        bool post() override;
    };
}
