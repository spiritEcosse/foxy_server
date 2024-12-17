#include "TwitterClient.h"

#include <cpr/cpr.h>
#include "fmt/format.h"
#include <string>
#include "TransparentStringHash.h"
#include <drogon/drogon.h>
#include <models/Tweet.h>
#include "cuuid.h"

namespace api::v1 {
    std::string TwitterClient::auth() {
        return auth(apiCreatePost);
    }

    bool TwitterClient::setPostId(const cpr::Response& response, const Json::Value& jsonResponse, Tweet* tweet) const {
        if(!fieldIsMember("data", response, jsonResponse) || !fieldIsMember("id", response, jsonResponse["data"]))
            return false;
        tweet->postId = jsonResponse["data"]["id"].asString();
        return true;
    }

    bool TwitterClient::uploadMediaImage(const Tweet* tweet) {
        std::vector<SharedFileTransferInfo> medias;
        std::ranges::copy_if(tweet->media, std::back_inserter(medias), [](const auto& mediaItem) {
            return !mediaItem->isVideo();
        });

        // If no images, return true (no images to upload)
        if(std::ranges::empty(medias))
            return true;

        // Create a MultiPerform object
        cpr::MultiPerform multiplePerform;

        // Store shared pointers to sessions to prevent premature destruction
        std::vector<std::shared_ptr<cpr::Session>> sessions;
        sessions.reserve(medias.size());

        // Create sessions for each media item
        std::ranges::transform(medias, std::back_inserter(sessions), [this, &multiplePerform](const auto& media) {
            auto session = std::make_shared<cpr::Session>();
            session->SetUrl(cpr::Url{apiUploadMedia});
            session->SetHeader({{"Authorization", auth(apiUploadMedia)}});
            session->SetMultipart({{"media", cpr::File{media->getFileName()}}, {"media_category", "tweet_image"}});
            multiplePerform.AddSession(session);
            return session;
        });

        // Perform all requests
        const std::vector<cpr::Response> responses = multiplePerform.Post();
        return checkResponses(responses) && saveMediaIdString(responses, medias);
    }

    bool TwitterClient::uploadMediaVideo(const Tweet* tweet) {
        // Filter out video files from media
        std::vector<SharedFileTransferInfo> medias;
        std::ranges::copy_if(tweet->media, std::back_inserter(medias), [](const auto& mediaItem) {
            return mediaItem->isVideo();
        });

        // If no videos, return true (no videos to upload)
        if(std::ranges::empty(medias))
            return true;

        cpr::MultiPerform multiplePerform;

        // INIT: First step of Twitter's chunked video upload
        std::vector<std::shared_ptr<cpr::Session>> initSessions;
        initSessions.reserve(medias.size());

        std::ranges::transform(medias, std::back_inserter(initSessions), [this, &multiplePerform](const auto& media) {
            auto session = std::make_shared<cpr::Session>();
            session->SetUrl(cpr::Url{apiUploadMedia});
            session->SetHeader({{"Authorization", auth(apiUploadMedia, "POST")}});

            // Prepare video file info for INIT
            session->SetMultipart({{"command", "INIT"},
                                   {"media_type", media->getContentType()},
                                   {"total_bytes", std::to_string(static_cast<int>(media->getSize()))},
                                   {"media_category", "tweet_video"}});
            multiplePerform.AddSession(session);
            return session;
        });

        // Execute INIT requests
        const auto initResponses = multiplePerform.Post();

        if(!checkResponses(initResponses, 202))
            return false;

        // save media_id_string to media
        saveMediaIdString(initResponses, medias);

        // APPEND: Upload video in chunks
        cpr::MultiPerform multiplePerformAppend;
        std::vector<std::shared_ptr<cpr::Session>> appendSessions;
        std::vector<std::shared_ptr<std::vector<char>>> allFileContents;
        allFileContents.reserve(medias.size());

        for(const auto& media: medias) {
            if(!media)
                continue;
            // Safely get file content
            auto fileContent = media->getFileContent();
            if(fileContent->empty())
                continue;

            std::string externalId = media->getExternalId();
            std::filesystem::path fileName = media->getFileName();
            if(externalId.empty() || fileName.empty())
                continue;
            constexpr size_t chunkSize = 1048576;  // 1MB chunks
            allFileContents.push_back(fileContent);

            size_t segmentIndex = 0;
            const size_t fileSize = fileContent->size();
            for(size_t offset = 0; offset < fileSize; offset += chunkSize) {
                const size_t currentChunkSize = std::min(chunkSize, fileSize - offset);
                assert(fileSize >= offset + currentChunkSize);
                auto session = std::make_shared<cpr::Session>();
                session->SetUrl(cpr::Url{apiUploadMedia});
                session->SetHeader({{"Authorization", auth(apiUploadMedia)}});

                try {
                    // Use ptrdiff_t for iterator difference to avoid narrowing conversion
                    auto start = fileContent->begin() + static_cast<std::ptrdiff_t>(offset);
                    auto end = std::min(fileContent->end(),
                                        fileContent->begin() + static_cast<std::ptrdiff_t>(offset + currentChunkSize));

                    session->SetMultipart({{"command", "APPEND"},
                                           {"media_id", externalId},
                                           {"segment_index", std::to_string(segmentIndex)},
                                           {"media", cpr::Buffer{start, end, std::move(fileName)}}});

                    multiplePerformAppend.AddSession(session);
                    appendSessions.push_back(session);
                } catch(const std::exception& e) {
                    // Log or handle the exception
                    std::cerr << "Error creating session: " << e.what() << std::endl;
                }
                segmentIndex++;
            }
        }

        // Execute APPEND requests

        if(!checkResponses(multiplePerformAppend.Post(), 204))
            return false;

        // FINALIZE: Mark video upload as complete
        cpr::MultiPerform multiplePerformFin;
        std::vector<std::shared_ptr<cpr::Session>> finalizeSessions;
        finalizeSessions.reserve(medias.size());

        std::ranges::transform(medias,
                               std::back_inserter(finalizeSessions),
                               [this, &multiplePerformFin](const auto& media) {
                                   auto session = std::make_shared<cpr::Session>();
                                   session->SetUrl(cpr::Url{apiUploadMedia});
                                   session->SetHeader({{"Authorization", auth(apiUploadMedia, "POST")}});
                                   session->SetMultipart(
                                       {{"command", "FINALIZE"}, {"media_id", media->getExternalId()}});
                                   multiplePerformFin.AddSession(session);
                                   return session;
                               });

        // Execute FINALIZE requests
        return checkResponses(multiplePerformFin.Post());
    }

    bool TwitterClient::uploadMedia(const Tweet* tweet) {
        std::future<bool> imageUploadFuture = std::async(std::launch::async, [this, tweet]() {
            return uploadMediaImage(tweet);
        });

        std::future<bool> videoUploadFuture = std::async(std::launch::async, [this, tweet]() {
            return uploadMediaVideo(tweet);
        });
        const bool videoUploadResult = videoUploadFuture.get();
        const bool imageUploadResult = imageUploadFuture.get();
        return videoUploadResult && imageUploadResult;
    }

    std::string
    TwitterClient::auth(const std::string_view url, const std::string_view method, const TransparentMap& params) {
        const auto now = std::chrono::system_clock::now().time_since_epoch();
        const auto now_in_seconds = std::chrono::duration_cast<std::chrono::seconds>(now).count();
        std::string oauth_timestamp = std::to_string(now_in_seconds);
        std::map<std::string, std::string, std::less<>> oauthParams = {
            {"oauth_consumer_key", apiKey},
            {"oauth_nonce", cuuid()},
            {"oauth_signature_method", "HMAC-SHA1"},
            {"oauth_timestamp", oauth_timestamp},
            {"oauth_token", accessToken},
            {"oauth_version", "1.0"},
        };
        std::ranges::transform(params, std::inserter(oauthParams, oauthParams.end()), [](const auto& pair) {
            return pair;  // Return the key-value pair unchanged
        });

        oauthParams["oauth_signature"] =
            calculateOAuthSignature(method, url, oauthParams, apiSecretKey, accessTokenSecret);

        std::vector<std::string> parts;
        parts.reserve(oauthParams.size());
        std::ranges::transform(oauthParams, std::back_inserter(parts), [](const auto& pair) {
            return fmt::format(R"({}="{}")", pair.first, urlEncode(pair.second));
        });

        return fmt::format("OAuth {}", fmt::join(parts, ", "));
    }
}
