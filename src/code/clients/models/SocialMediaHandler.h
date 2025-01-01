#pragma once

#include "Pin.h"

namespace api::v1 {
    class SocialMediaHandler {
    public:
        void handleRow(const auto& row);

    private:
        std::vector<std::string> processMediaUrls(const Json::Value& mediaList, size_t maxImages = 4);
        void saveToDatabase(const std::string& platform, const std::string& postId, const std::string& itemId);
    };
}
