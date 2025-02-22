#include "PinterestClient.h"
#include "Pin.h"

namespace api::v1 {
    std::string PinterestClient::apiUploadMedia = fmt::format("{}/v5/media", PINTEREST_API_HOST);
    std::string PinterestClient::apiCreatePost = fmt::format("{}/v5/pins", PINTEREST_API_HOST);
    std::string PinterestClient::tokenUrl = fmt::format("{}/v5/oauth/token", PINTEREST_API_HOST);

    std::string PinterestClient::auth() const {
        return fmt::format("Bearer {}", accessToken);
    }

    std::string PinterestClient::getAccessToken() const {
        return accessToken;
    }

    bool PinterestClient::setAccessToken() {
        if(!accessToken.empty())
            return true;

        cpr::Payload payload{{"refresh_token", std::string(refreshToken)}, {"grant_type", "refresh_token"}};
        const cpr::Response response = Post(
            cpr::Url{tokenUrl},
            std::move(payload),
            cpr::Header{{"Content-Type", "application/x-www-form-urlencoded"},
                        {"Authorization", "Basic " + Base64::Encode(std::format("{}:{}", clientId, clientSecret))}});

        if(!checkResponses(std::vector{response}))
            return false;

        Json::Value jsonResponse;

        if(!parseJson(response, jsonResponse) || !fieldIsMember("access_token", response, jsonResponse))
            return false;
        accessToken = jsonResponse["access_token"].asString();
        return true;
    }

    bool PinterestClient::uploadVideos(const Pin* pin) const {
        if(accessToken.empty()) {
            sentryHelper(std::runtime_error("Pinterest Access token is empty"), "PinterestClient::uploadVideos");
            return false;
        }

        if(pin->videos.empty())
            return false;

        cpr::MultiPerform multiplePerform;

        std::vector<std::shared_ptr<cpr::Session>> sessionsInit;
        sessionsInit.reserve(pin->videos.size());

        std::ranges::transform(pin->videos,
                               std::back_inserter(sessionsInit),
                               [this, &multiplePerform]([[maybe_unused]] const auto& media) {
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
