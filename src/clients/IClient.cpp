#include "IClient.h"

#include "models/Tweet.h"
#include "models/Pin.h"

namespace api::v1 {

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
