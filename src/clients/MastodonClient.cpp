#include "MastodonClient.h"

#include <sentryHelper.h>
#include <drogon/drogon.h>
#include <fmt/core.h>
#include <cpr/cpr.h>
#include "models/Tweet.h"  // must be because of it : ClientType::clientName
#include "models/Pin.h"  // must be because of it : ClientType::clientName
#include <algorithm>

namespace api::v1 {
    MastodonClient::MastodonClient(const Json::Value& mediaJson) : media(transformMedia(mediaJson)) {}

    bool MastodonClient::downloadMedia() {
        // Create a MultiPerform object
        cpr::MultiPerform multiplePerform;

        // Store shared pointers to sessions to prevent premature destruction
        std::vector<std::shared_ptr<cpr::Session>> sessions;

        // Create sessions for each media item
        std::ranges::transform(media, std::back_inserter(sessions), [&multiplePerform](const auto& mediaItem) {
            auto session = std::make_shared<cpr::Session>();
            session->SetUrl(cpr::Url{mediaItem->getUrl()});
            multiplePerform.AddSession(session);
            return session;
        });
        // Perform all requests
        std::vector<cpr::Response> responses = multiplePerform.Get();

        if(const bool responsesIsOk = checkResponses(responses); !responsesIsOk) {
            return false;
        }

        // Process responses
        for(size_t idx = 0; idx < responses.size(); ++idx) {
            if(!media[idx]->saveFile(std::move(responses[idx].text))) {
                return false;
            }
        }
        return true;
    }

    std::vector<SharedFileTransferInfo> MastodonClient::transformMedia(const Json::Value& mediaJson) {
        // Find the minimum number of media items to process
        // This ensures we don't exceed the maximum allowed media items for either Tweets or Pins
        constexpr size_t maxMediaItems =
            std::max(static_cast<size_t>(Tweet::maxMediaItems), static_cast<size_t>(Pin::maxMediaItems));
        const size_t numItems = std::min(maxMediaItems, static_cast<size_t>(mediaJson.size()));

        // Create an empty vector to store FileTransferInfo objects
        // Use reserve to optimize memory allocation
        std::vector<SharedFileTransferInfo> mediaUrls;
        mediaUrls.reserve(numItems);

        // Iterate through the mediaJson
        std::ranges::transform(mediaJson | std::ranges::views::take(numItems),
                               std::back_inserter(mediaUrls),
                               [](const Json::Value& objJson) {
                                   const std::string mediaUrl = objJson["url"].asString();
                                   return std::make_shared<FileTransferInfo>(
                                       fmt::format("{}?twic=v1/cover=4000", mediaUrl),
                                       mediaUrl.substr(mediaUrl.find_last_of('/') + 1),
                                       objJson["type"].asString());
                               });
        return mediaUrls;
    }
}
