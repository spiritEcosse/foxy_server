#pragma once
#include "IClientImpl.h"

namespace api::v1 {
    template<typename ClientType, typename PostType>
    class IClient : public IClientImpl {
    public:
        using IClientImpl::IClientImpl;
        virtual bool post(PostType* postType);
        virtual std::string auth() = 0;
        // Json::Value requestCurlOauth(const std::string& url,
        //                              const std::string& method,
        //                              const std::map<std::string, std::string, std::less<>>& oauthParams = {});
    };
}
