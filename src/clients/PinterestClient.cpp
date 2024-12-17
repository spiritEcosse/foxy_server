#include "PinterestClient.h"
#include "models/Pin.h"

namespace api::v1 {
    std::string PinterestClient::apiUploadMedia = fmt::format("{}/v5/media", PINTEREST_API_HOST);
    std::string PinterestClient::apiCreatePost = fmt::format("{}/v5/pins", PINTEREST_API_HOST);

    std::string PinterestClient::auth() {
        return fmt::format("Bearer {}", accessToken);
    }

    bool PinterestClient::uploadVideos(const Pin* pin) {
        std::vector<SharedFileTransferInfo> medias;
        std::ranges::copy_if(pin->media, std::back_inserter(medias), [](const auto& mediaItem) {
            return mediaItem->isVideo();
        });
        if(std::ranges::empty(medias))
            return true;

        cpr::MultiPerform multiplePerform;

        std::vector<std::shared_ptr<cpr::Session>> sessionsInit;
        sessionsInit.reserve(medias.size());

        std::ranges::transform(medias, std::back_inserter(sessionsInit), [this, &multiplePerform](const auto& media) {
            auto session = std::make_shared<cpr::Session>();
            session->SetUrl(cpr::Url{apiUploadMedia});
            session->SetHeader({{"Authorization", auth()}, {"Content-Type", "application/json"}});
            session->SetBody(cpr::Body{R"({"media_type":"video"})"});
            multiplePerform.AddSession(session);
            return session;
        });
        const auto responses = multiplePerform.Post();
        if(!checkResponses(responses, 201))
            return false;

        if(!saveMediaIdString(responses, medias))
            return false;

        cpr::MultiPerform multipleUpload;
        std::vector<std::shared_ptr<cpr::Session>> sessionsUpload;
        sessionsUpload.reserve(medias.size());

        std::ranges::transform(medias, std::back_inserter(sessionsUpload), [this, &multipleUpload](const auto& media) {
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
