#include "PinterestClient.h"
#include "Pin.h"

namespace api::v1 {
    std::string PinterestClient::apiUploadMedia = fmt::format("{}/v5/media", PINTEREST_API_HOST);
    std::string PinterestClient::apiCreatePost = fmt::format("{}/v5/pins", PINTEREST_API_HOST);

    std::string PinterestClient::auth() const {
        return fmt::format("Bearer {}", accessToken);
    }

    bool PinterestClient::uploadVideos(const Pin* pin) const {
        if(pin->videos.empty())
            return false;

        cpr::MultiPerform multiplePerform;

        std::vector<std::shared_ptr<cpr::Session>> sessionsInit;
        sessionsInit.reserve(pin->videos.size());

        std::ranges::transform(pin->videos,
                               std::back_inserter(sessionsInit),
                               [this, &multiplePerform](const auto& media) {
                                   auto session = std::make_shared<cpr::Session>();
                                   session->SetUrl(cpr::Url{apiUploadMedia});
                                   session->SetHeader(getHttpHeaders());
                                   session->SetBody(cpr::Body{R"({"media_type":"video"})"});
                                   multiplePerform.AddSession(session);
                                   return session;
                               });
        if(const auto responses = multiplePerform.Post();
           !checkResponses(responses, 201) || !saveMediaIdString(responses, pin->videos))
            return false;

        cpr::MultiPerform multipleUpload;
        std::vector<std::shared_ptr<cpr::Session>> sessionsUpload;
        sessionsUpload.reserve(pin->videos.size());

        std::ranges::transform(pin->videos,
                               std::back_inserter(sessionsUpload),
                               [this, &multipleUpload](const auto& media) {
                                   auto session = std::make_shared<cpr::Session>();
                                   const auto& response = media->getResponse();
                                   session->SetUrl(cpr::Url{response["upload_url"].asString()});

                                   cpr::Multipart multipart{};

                                   const auto& uploadParams = response["upload_parameters"];
                                   for(const auto& paramName: uploadParams.getMemberNames()) {
                                       multipart.parts.emplace_back(paramName, uploadParams[paramName].asString());
                                   }
                                   multipart.parts.emplace_back("file", cpr::File{media->getFileName()});
                                   session->SetMultipart(multipart);
                                   multipleUpload.AddSession(session);
                                   return session;
                               });
        return checkResponses(multipleUpload.Post(), 204);
    }

    bool PinterestClient::setPostId(const cpr::Response& response, const Json::Value& jsonResponse, Pin* pin) const {
        if(!fieldIsMember("id", response, jsonResponse))
            return false;
        pin->postId = jsonResponse["id"].asString();
        return true;
    }
}
