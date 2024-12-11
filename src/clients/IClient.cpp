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
        if(!parseJson(response, jsonResponse))
            return false;

        if(!fieldIsMember("data", response, jsonResponse) || !fieldIsMember("id", response, jsonResponse["data"]))
            return false;
        postType->postId = jsonResponse["data"]["id"].asString();
        return true;
    }

    template class IClient<PinterestClient, Pin>;
    template class IClient<TwitterClient, Tweet>;
}
