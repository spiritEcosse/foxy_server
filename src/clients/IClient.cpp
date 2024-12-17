#include "IClient.h"

#include "models/Tweet.h"
#include "models/Pin.h"

namespace api::v1 {
    template<typename ClientType, typename PostType>
    bool IClient<ClientType, PostType>::saveMediaIdString(const std::vector<cpr::Response>& responses,
                                                          const std::vector<SharedFileTransferInfo>& medias) {
        for(size_t i = 0; const auto& media: medias) {
            const auto& response = responses[i];

            // Parse JSON response using JsonCpp
            Json::Value jsonResponse;

            if(!parseJson(response, jsonResponse))
                return false;

            media->setResponse(jsonResponse);
            if(!fieldIsMember(ClientType::media_id, response, jsonResponse))
                return false;
            media->setExternalId(jsonResponse[ClientType::media_id].asString());
            std::cout << media->getExternalId() << " : " << media->getFileName() << std::endl;
            ++i;
        }
        return true;
    }

    template<typename ClientType, typename PostType>
    bool IClient<ClientType, PostType>::post(PostType* postType) {
        const cpr::Response response =
            Post(cpr::Url{ClientType::apiCreatePost},
                 cpr::Body{postType->toJson()},
                 cpr::Header{{"Content-Type", "application/json"}, {"Authorization", auth()}});
        if(!checkResponses(std::vector{response}, 201))
            return false;

        Json::Value jsonResponse;
        return parseJson(response, jsonResponse) && setPostId(response, jsonResponse, postType);
    }

    template class IClient<PinterestClient, Pin>;
    template class IClient<TwitterClient, Tweet>;
}
