#pragma once

#include "SocialMediaType.h"

#ifndef PINTEREST_BOARD_ID
#error PINTEREST_BOARD_ID must be defined at compile-time
#endif

namespace api::v1 {
    class Pin final : public SocialMediaType<PinterestClient, Pin> {
        static constexpr std::string_view boardId = PINTEREST_BOARD_ID;

    public:
        static constexpr int maxDescriptionSize = 800;
        static constexpr int maxTitleSize = 100;
        static constexpr int maxMediaItems = 5;

        Pin(int itemId,
            const std::string_view& title,
            const std::string_view& slug,
            const std::string_view& description,
            const std::vector<SharedFileTransferInfo>& media,
            const Json::Value& tags);

        std::string toJson() override;
        bool post() override;
        bool postVideos() const;
    };
}
