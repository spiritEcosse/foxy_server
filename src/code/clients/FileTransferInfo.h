#pragma once
#include "Base64.h"
#include "BaseClass.h"

#include <MediaModel.h>
#include <fstream>
#include <string>
#include <filesystem>
#include <drogon/drogon.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

constexpr size_t MAX_FILE_SIZE = 20 * 1024 * 1024;

namespace api::v1 {
    class Pin;

    class FileTransferInfo final : public BaseClass {
        std::string url;
        std::string fileName;
        std::string externalIdPin;
        std::string externalIdTweet;
        std::string type;
        std::string contentType;
        Json::Value responsePin;
        Json::Value responseTweet;

    public:
        FileTransferInfo(std::string url, std::string fileName, std::string type, std::string contentType) :
            url(std::move(url)), fileName(std::move(fileName)), type(std::move(type)),
            contentType(std::move(contentType)) {}

        FileTransferInfo(FileTransferInfo&& other) noexcept :
            url(std::move(other.url)), fileName(std::move(other.fileName)),
            externalIdPin(std::move(other.externalIdPin)), externalIdTweet(std::move(other.externalIdTweet)),
            type(std::move(other.type)), contentType(std::move(other.contentType)),
            responsePin(std::move(other.responsePin)), responseTweet(std::move(other.responseTweet)) {}

        ~FileTransferInfo() override {
            try {
                std::filesystem::remove(fileName);
            } catch(const std::filesystem::filesystem_error& e) {
                LOG_ERROR << "Error removing file: " << e.what();
            }
        }

        [[nodiscard]] Json::Value getResponse() const {
            return responsePin;
        }

        template<typename PostType>
        void setResponse(const Json::Value& originalResponse) {
            if constexpr(std::is_same_v<PostType, Pin>)
                responsePin = originalResponse;
            else
                responseTweet = originalResponse;
        }

        [[nodiscard]] bool saveFile(std::string&& content) const {
            std::ofstream outputFile(fileName, std::ios::binary);
            if(!outputFile)
                return false;

            outputFile.write(std::move(content).c_str(), content.size());

            return outputFile.good();
        }

        [[nodiscard]] size_t getSize() const {
            try {
                std::ifstream file(fileName, std::ios::binary | std::ios::ate);
                if(!file) {
                    LOG_ERROR << "Failed to open file: " << fileName << " Error: " << std::strerror(errno);
                    return 0;
                }
                return file.tellg();
            } catch(const std::exception& e) {
                LOG_ERROR << "Exception in getSize: " << e.what();
                return 0;
            }
        }

        [[nodiscard]] std::shared_ptr<std::vector<char>> getFileContent() const {
            try {
                std::ifstream file(fileName, std::ios::binary);
                if(!file) {
                    LOG_ERROR << "Failed to open file: " << fileName << " Error: " << std::strerror(errno);
                    return nullptr;
                }

                file.seekg(0, std::ios::end);
                std::streampos fileSize = file.tellg();
                file.seekg(0, std::ios::beg);

                if(fileSize > MAX_FILE_SIZE) {
                    LOG_ERROR << "File too large: " << fileName << " Size: " << fileSize;
                    return nullptr;
                }

                auto buffer = std::make_shared<std::vector<char>>(static_cast<size_t>(fileSize));
                if(!file.read(buffer->data(), fileSize)) {
                    LOG_ERROR << "Failed to read file: " << fileName << " Error: " << std::strerror(errno);
                    return nullptr;
                }

                return buffer;
            } catch(const std::exception& e) {
                LOG_ERROR << "Exception in getFileContent: " << e.what();
                return nullptr;
            }
        }

        [[nodiscard]] std::string getFileName() const {
            return fileName;
        }

        template<typename PostType>
        [[nodiscard]] const std::string& getExternalId() const {
            if constexpr(std::is_same_v<PostType, Pin>)
                return externalIdPin;
            else
                return externalIdTweet;
        }

        [[nodiscard]] std::string getBase64ContentOfFile() const {
            std::ifstream file(fileName, std::ios::binary);
            if(!file)
                return {};

            return Base64::Encode(
                std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()));
        }

        [[nodiscard]] const std::string& getUrl() const {
            return url;
        }

        template<typename PostType>
        void setExternalId(std::string&& value) {
            if constexpr(std::is_same_v<PostType, Pin>)
                externalIdPin = std::move(value);
            else
                externalIdTweet = std::move(value);
        }

        [[nodiscard]] const std::string& getContentType() const {
            return contentType;
        }

        [[nodiscard]] bool isVideo() const {
            return type == MediaType::VIDEO;
        }
    };

    using SharedFileTransferInfo = decltype(std::shared_ptr<FileTransferInfo>(nullptr));

}
