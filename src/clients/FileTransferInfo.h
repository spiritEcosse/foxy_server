#pragma once
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
    class FileTransferInfo final : public BaseClass {
        std::string url;
        std::string fileName;
        std::string externalId;
        std::string type;
        std::string contentType;

    public:
        FileTransferInfo(std::string url, std::string fileName, std::string type) :
            url(std::move(url)), fileName(std::move(fileName)), type(std::move(type)) {
            contentType = createContentType();
        }

        FileTransferInfo(FileTransferInfo&& other) noexcept :
            url(std::move(other.url)), fileName(std::move(other.fileName)), externalId(std::move(other.externalId)),
            type(std::move(other.type)), contentType(std::move(other.contentType)) {}

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

        [[nodiscard]] bool saveFile(std::string&& content) const {
            std::ofstream outputFile(fileName, std::ios::binary);
            if(!outputFile)
                return false;

            outputFile.write(content.c_str(), content.size());

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

        [[nodiscard]] const std::string& getExternalId() const {
            return externalId;
        }

        [[nodiscard]] std::string getBase64Response() const {
            // Open the file in binary mode
            std::ifstream file(fileName, std::ios::binary);
            if(!file) {
                return "";  // Return empty string if file cannot be opened
            }

            // Read the entire file content
            std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            // If file is empty, return empty string
            if(fileContent.empty()) {
                return "";
            }

            // OpenSSL Base64 encoding
            BIO* bio = BIO_new(BIO_s_mem());
            BIO* b64 = BIO_new(BIO_f_base64());

            bio = BIO_push(b64, bio);
            BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

            BIO_write(bio, fileContent.data(), fileContent.size());
            BIO_flush(bio);

            BUF_MEM* mem_ptr = nullptr;
            BIO_get_mem_ptr(bio, &mem_ptr);

            std::string encodedData(mem_ptr->data, mem_ptr->length);

            BIO_free_all(bio);

            return encodedData;
        }

        [[nodiscard]] const std::string& getUrl() const {
            return url;
        }

        void setExternalId(std::string&& value) {
            externalId = std::move(value);
        }

        [[nodiscard]] const std::string& getContentType() const {
            return contentType;
        }

        std::string createContentType() {
            // Extract the file extension from fileName
            const size_t pos = fileName.find_last_of('.');
            std::string extension;
            if(pos != std::string::npos) {
                extension = fileName.substr(pos + 1);  // Get the extension without the dot
            }

            // Concatenate type and extension to form contentType
            if(!type.empty() && !extension.empty()) {
                return fmt::format("{}/{}", type, extension);
            }

            // Fallback for missing type or extension
            return "";
        }

        // function to detect is it video or not
        [[nodiscard]] bool isVideo() const {
            return type == MediaType::VIDEO;
        }
    };

    using SharedFileTransferInfo = decltype(std::shared_ptr<FileTransferInfo>(nullptr));

}
