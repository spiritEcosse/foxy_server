#pragma once

#include "SocialMediaType.h"

namespace api::v1 {
    class Pin final : public SocialMediaType<PinterestClient, Pin> {
        std::string boardId;

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
    };
}
