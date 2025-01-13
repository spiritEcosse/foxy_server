#pragma once

#include "SocialMediaType.h"

namespace api::v1 {
    class Tweet final : public SocialMediaType<TwitterClient, Tweet> {

    public:
        static constexpr int maxDescriptionSize = 0;
        static constexpr int maxTitleSize = 280;
        static constexpr int maxMediaItems = 4;

        Tweet(int itemId,
              const std::string_view& title,
              const std::string_view& slug,
              const std::string_view& description,
              const std::vector<SharedFileTransferInfo>& media,
              const Json::Value& tags);

        std::string toJson() const override;
        bool post() override;
    };
}
