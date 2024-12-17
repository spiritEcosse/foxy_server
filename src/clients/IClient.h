#pragma once
#include "IClientImpl.h"

namespace api::v1 {
    template<typename ClientType, typename PostType>
    class IClient : public IClientImpl {
    public:
        using IClientImpl::IClientImpl;
        virtual bool post(PostType* postType);
        virtual std::string auth() = 0;
        virtual bool
        setPostId(const cpr::Response& response, const Json::Value& jsonResponse, PostType* tweet) const = 0;
        static bool saveMediaIdString(const std::vector<cpr::Response>& responses,
                                      const std::vector<SharedFileTransferInfo>& medias);
    };
}
