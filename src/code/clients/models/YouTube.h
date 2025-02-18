#pragma once

#include "SocialMediaType.h"
#include "env.h"

namespace api::v1 {
    class YouTube final : public SocialMediaType<YouTubeClient, YouTube> {
        [[nodiscard]] bool postVideo();
        [[nodiscard]] bool postVideos();

    public:
        static constexpr int maxDescriptionSize = 5000;
        static constexpr int maxTitleSize = 100;
        static constexpr int maxMediaItems = 100000;
        SharedFileTransferInfo video;

        YouTube(int itemId,
                const std::string_view& titleOriginal,
                const std::string_view& slug,
                const std::string_view& description,
                const std::vector<SharedFileTransferInfo>& media,
                const Json::Value& tags);

        YouTube(int itemId,
                const std::string_view& titleOriginal,
                const std::string_view& slug,
                const std::string_view& description,
                const SharedFileTransferInfo& video,
                const std::vector<std::string>& tags);

        std::string toJson() const override;
        bool post() override;
    };
}
