#include "YouTubeClient.h"
#include "YouTube.h"

namespace api::v1 {
    std::string YouTubeClient::auth() const {
        return fmt::format("Bearer {}", key);
    }

    bool YouTubeClient::uploadVideo(YouTube* post) const {
        // Prepare the video file for upload
        // Step 1: Initiate the upload
        const auto initResponse = Post(cpr::Url{apiUploadMedia},
                                       cpr::Body{post->toJson()},
                                       cpr::Parameters{{"uploadType", "resumable"}, {"part", "snippet,status"}});

        // print initResponse
        std::cout << initResponse.text << std::endl;

        if(!checkResponses(std::vector{initResponse}))
            return false;

        cpr::Url uploadUrl;
        // Get the upload URL from the response
        if(const auto location_iter = initResponse.header.find("Location");
           location_iter != initResponse.header.end()) {
            uploadUrl = location_iter->second;
        } else {
            std::cerr << "Location header not found.\n";
            return false;
        }

        // Step 2: Upload the video file
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
