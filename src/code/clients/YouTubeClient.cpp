#include "clients/YouTubeClient.h"

#include "clients/models/YouTube.h"

namespace api::v1 {
    std::string YouTubeClient::auth() const {
        return fmt::format("Bearer {}", accessToken);
    }

    std::string YouTubeClient::getAccessToken() const {
        return accessToken;
    }

    bool YouTubeClient::setAccessToken() {
        if(!accessToken.empty())
            return true;

        cpr::Payload payload{{"client_id", std::string(clientId)},
                             {"client_secret", std::string(clientSecret)},
                             {"refresh_token", std::string(refreshToken)},
                             {"grant_type", "refresh_token"}};
        const cpr::Response response = Post(cpr::Url{tokenUrl},
                                            std::move(payload),
                                            cpr::Header{{"Content-Type", "application/x-www-form-urlencoded"}});

        if(!checkResponses(std::vector{response}))
            return false;

        Json::Value jsonResponse;

        if(!parseJson(response, jsonResponse) || !fieldIsMember("access_token", response, jsonResponse))
            return false;
        accessToken = jsonResponse["access_token"].asString();
        return true;
    }

    bool YouTubeClient::uploadVideo(YouTube* post) const {
        if(accessToken.empty()) {
            sentryHelper(std::runtime_error("YouTube Access token is empty"), "YouTubeClient::uploadVideos");
            return false;
        }

        const auto initResponse = Post(cpr::Url{apiUploadMedia},
                                       getHttpHeaders(),
                                       cpr::Body{post->toJson()},
                                       cpr::Parameters{{"uploadType", "resumable"}, {"part", "snippet,status"}});

        if(!checkResponses(std::vector{initResponse}))
            return false;

        cpr::Url uploadUrl;
        if(const auto location_iter = initResponse.header.find("Location");
           location_iter != initResponse.header.end()) {
            uploadUrl = location_iter->second;
        } else {
            sentryHelper(std::runtime_error("Location header not found."), "YouTubeClient::checkResponses");
            return false;
        }

        const auto uploadResponse =
            Put(cpr::Url{uploadUrl},
                cpr::Header{{"Authorization", auth()}, {"Content-Type", post->video->getContentType()}},
                cpr::File{post->video->getFileName()});

        Json::Value jsonResponse;
        return checkResponses(std::vector{uploadResponse}) && parseJson(uploadResponse, jsonResponse) &&
               setPostId(uploadResponse, jsonResponse, post);
    }

    bool YouTubeClient::setPostId(const cpr::Response& response, const Json::Value& jsonResponse, YouTube* post) const {
        if(!fieldIsMember("id", response, jsonResponse))
            return false;
        post->postId = jsonResponse["id"].asString();
        return true;
    }
}
