#include "PinterestClient.h"
#include "models/Pin.h"

namespace api::v1 {
    std::string PinterestClient::auth() {
        return fmt::format("Bearer {}", accessToken);
    }

    bool PinterestClient::setPostId(const cpr::Response& response, const Json::Value& jsonResponse, Pin* pin) const {
        if(!fieldIsMember("id", response, jsonResponse))
            return false;
        pin->postId = jsonResponse["id"].asString();
        return true;
    }

}
