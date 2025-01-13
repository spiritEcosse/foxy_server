#pragma once
#include "Base64.h"
#include "BaseClass.h"

#include <MediaModel.h>
#include <fstream>
#include <string>
#include <filesystem>
#include <iostream>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

constexpr size_t MAX_FILE_SIZE = 20 * 1024 * 1024;  // 20 MB in bytes

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

        // Destructor to remove the file when the object is destroyed
        ~FileTransferInfo() override {
            try {
                // Use std::filesystem to remove the file
                std::filesystem::remove(fileName);
            } catch(const std::filesystem::__cxx11::filesystem_error& e) {
                // Optional: Log the error or handle it as appropriate for your application
                // For example, you might want to log the error or use a logging framework
                std::cerr << "Error removing file: " << e.what() << std::endl;
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
                    std::cerr << "Failed to open file: " << fileName << " Error: " << std::strerror(errno) << std::endl;
                    return 0;
                }
                return file.tellg();
            } catch(const std::exception& e) {
                std::cerr << "Exception in getSize: " << e.what() << std::endl;
                return 0;
            }
        }

        [[nodiscard]] std::shared_ptr<std::vector<char>> getFileContent() const {
            try {
                std::ifstream file(fileName, std::ios::binary);
                if(!file) {
                    std::cerr << "Failed to open file: " << fileName << " Error: " << std::strerror(errno) << std::endl;
                    return nullptr;
                }

                file.seekg(0, std::ios::end);
                std::streampos fileSize = file.tellg();
                file.seekg(0, std::ios::beg);

                if(fileSize > MAX_FILE_SIZE) {
                    std::cerr << "File too large: " << fileName << " Size: " << fileSize << std::endl;
                    return nullptr;
                }

                auto buffer = std::make_shared<std::vector<char>>(static_cast<size_t>(fileSize));
                if(!file.read(buffer->data(), fileSize)) {
                    std::cerr << "Failed to read file: " << fileName << " Error: " << std::strerror(errno) << std::endl;
                    return nullptr;
                }

                return buffer;
            } catch(const std::exception& e) {
                std::cerr << "Exception in getFileContent: " << e.what() << std::endl;
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

        // function to detect is it video or not
        [[nodiscard]] bool isVideo() const {
            return type == MediaType::VIDEO;
        }
    };

    using SharedFileTransferInfo = decltype(std::shared_ptr<FileTransferInfo>(nullptr));

}
