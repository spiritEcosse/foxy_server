#pragma once
#include <json/json.h>
#include "FileTransferInfo.h"

#include <IClientImpl.h>

namespace api::v1 {
    class MastodonClient final : public IClientImpl {
    public:
        using IClientImpl::IClientImpl;

        explicit MastodonClient(const Json::Value& mediaJson);
        bool downloadMedia();

        // Properties
        std::vector<SharedFileTransferInfo> media;

    private:
        [[nodiscard]] static std::vector<SharedFileTransferInfo> transformMedia(const Json::Value& mediaJson);
    };
}
