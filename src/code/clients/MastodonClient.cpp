#include "clients/MastodonClient.h"

#include <sentry_catcher/sentryHelper.h>
#include <drogon/drogon.h>
#include <fmt/core.h>
#include <cpr/cpr.h>
#include "clients/models/Tweet.h"  // must be because of it : ClientType::clientName
#include "clients/models/Pin.h"  // must be because of it : ClientType::clientName
#include <algorithm>

namespace api::v1 {
    MastodonClient::MastodonClient(const Json::Value& mediaJson) : media(transformMedia(mediaJson)) {}

    bool MastodonClient::downloadMedia() {
        cpr::MultiPerform multiplePerform;

        auto sessions = media | std::views::transform([&multiplePerform](const auto& mediaItem) {
                            auto session = std::make_shared<cpr::Session>();
                            session->SetUrl(cpr::Url{mediaItem->getUrl()});
                            multiplePerform.AddSession(session);
                            return session;
                        }) |
                        std::ranges::to<std::vector>();
        std::vector<cpr::Response> responses = multiplePerform.Get();

        if(!checkResponses(responses))
            return false;

        for(auto&& [response, mediaItem]: std::views::zip(responses, media)) {
            if(!mediaItem->saveFile(std::move(response.text)))
                return false;
        }
        return true;
    }

    std::vector<SharedFileTransferInfo> MastodonClient::transformMedia(const Json::Value& mediaJson) {
        return mediaJson | std::views::transform([](const Json::Value& objJson) {
                   const std::string mediaUrl = objJson["url"].asString();
                   return std::make_shared<FileTransferInfo>(fmt::format("{}?twic=v1/cover=2000x2000", mediaUrl),
                                                             mediaUrl.substr(mediaUrl.find_last_of('/') + 1),
                                                             objJson["type"].asString(),
                                                             objJson["content_type"].asString());
               }) |
               std::ranges::to<std::vector>();
    }
}
